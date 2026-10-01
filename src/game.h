#ifndef GAME_H
#define GAME_H

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "level.h"
#include "player.h"
#include "server.h"
#include "socket.h"

Player players[128];
/*
The players table element should be -1 if it's empty.
*/

pthread_mutex_t players_lock = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char* fmt, ...) {
    char msg[65];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    pthread_mutex_lock(&players_lock);
    for (int i = 0; i < 128; i++) {
        if (players[i].spawned && players[i].username != NULL) {
            send_message(players[i].sock, msg);
        }
    }
    pthread_mutex_unlock(&players_lock);
}

#define LIVE(p) ((p).username != NULL && (p).spawned)

void relay_pos_ort_locked(Player* from) {
    for (int i = 0; i < 128; i++) {
        if (!LIVE(players[i]) || &players[i] == from) continue;
        send_pos_ort(players[i].sock, from->id, from->x, from->y, from->z, (uint8_t)from->yaw,
                     (uint8_t)from->pitch);
    }
}

void broadcast_block(short x, short y, short z, uint8_t block) {
    pthread_mutex_lock(&players_lock);
    for (int i = 0; i < 128; i++) {
        if (LIVE(players[i])) send_block(players[i].sock, x, y, z, block);
    }
    pthread_mutex_unlock(&players_lock);
}

void* handle_player(void* arg) {
    int new_socket = (int)(intptr_t)arg;

    while (true) {
        // From here we handle client

        unsigned char buffer[131] = {0};

        ssize_t valread = read(new_socket, buffer, sizeof(buffer));

        if (valread <= 0) {
            perror("read");
            close(new_socket);
            return NULL;
        }

        // read_id(new_socket);

        Player new_player = {0};
        Player* me = NULL;
        new_player.sock = new_socket;

        if (buffer[0] == 0x00) {
            if (init_player((char*)buffer, &new_player) != 0) {
                printf("Player initialization failed");
                close(new_socket);
                return NULL;
            }

            pthread_mutex_lock(&players_lock);

            int slot = -1, taken = 0;

            for (int i = 0; i < 128; i++) {
                if (players[i].username == NULL) {
                    if (slot < 0) slot = i;
                } else if (strcmp(players[i].username, new_player.username) == 0)
                    taken = 1;
            }

            if (taken || slot < 0) {
                pthread_mutex_unlock(&players_lock);
                printf("Username '%s' already logged in\n", new_player.username);
                send_disconnect(new_socket, taken ? "Already logged in" : "Server is full");
                free(new_player.username);
                close(new_socket);
                return NULL;
            }

            new_player.id = slot;
            new_player.spawned = 0;
            new_player.x = level.sizeX / 2.0f;
            new_player.y = level.sizeY;
            new_player.z = level.sizeZ / 2.0f;
            players[slot] = new_player;
            me = &players[slot];
            pthread_mutex_unlock(&players_lock);

            send_server_identification(new_socket, "A Minecraft Server", "Welcome!");

            /*pthread_mutex_lock(&players_lock);
            players[new_player.id] = new_player;  // appending player to global table
            pthread_mutex_unlock(&players_lock);*/

            new_level(new_socket);
        } else {
            printf("A client connected but sent invalid data. Closing\n");
            close(new_socket);
            return NULL;
        }

        /*unsigned char b[100] = {0};
        ssize_t bsize = read(new_socket, b, sizeof(b));
        printf("%s\n", b);*/

        pthread_mutex_lock(&players_lock);
        me->spawned = 1;

        send_spawn(new_socket, -1, me->username, me->x, me->y, me->z, 0, 0);

        for (int i = 0; i < 128; i++) {
            Player* o = &players[i];
            if (!LIVE(*o) || o == me) continue;
            send_spawn(new_socket, o->id, o->username, o->x, o->y, o->z, (uint8_t)o->yaw,
                       (uint8_t)o->pitch);
            send_spawn(o->sock, me->id, me->username, me->x, me->y, me->z, 0, 0);
        }

        pthread_mutex_unlock(&players_lock);

        broadcast_message("&e%s joined the chat", me->username);

        while (true) {
            unsigned char buf[1] = {0};
            ssize_t bufsize = read(new_socket, buf, sizeof(buf));
            if (bufsize <= 0) break;

            int should_exit = 0;

            switch (buf[0]) {
                case 0x05: {  // Set block
                    unsigned char packet[8] = {0};
                    ssize_t bytes = read(new_socket, packet, sizeof(packet));
                    if (bytes <= 0) {
                        should_exit = 1;
                        break;
                    }

                    // recv_block((char*)packet, &new_player);
                    uint16_t x, y, z;
                    uint8_t block;
                    if (recv_block((char*)packet, &x, &y, &z, &block) == 0)
                        broadcast_block(x, y, z, block);
                } break;
                case 0x08: {  // Pos ort
                    unsigned char packet[9] = {0};
                    ssize_t bytes = read(new_socket, packet, sizeof(packet));
                    if (bytes <= 0) {
                        should_exit = 1;
                        break;
                    }

                    pthread_mutex_lock(&players_lock);
                    recv_pos_ort((char*)packet, me);
                    relay_pos_ort_locked(me);
                    pthread_mutex_unlock(&players_lock);
                } break;
                case 0x0d: {  // Message
                    unsigned char packet[65] = {0};
                    ssize_t bytes = read(new_socket, packet, sizeof(packet));
                    if (bytes <= 0) {
                        should_exit = 1;
                        break;
                    }

                    char received[64] = {0};
                    recv_message((char*)packet, &new_player, received);

                    char msg[64];
                    snprintf(msg, sizeof(msg), "%s: %s", new_player.username, received);

                    broadcast_message("%s", msg);

                } break;
                default:
                    printf("ERROR: player sent unknown packet %d\n", buf[0]);
                    break;
            }
            if (should_exit) break;
        }

        broadcast_message("&e%s left the chat", me->username);

        pthread_mutex_lock(&players_lock);
        int my_id = me->id;
        free(me->username);
        me->username = NULL;
        me->spawned = 0;
        me->id = -1;
        me->sock = -1;

        for (int i = 0; i < 128; i++) {
            if (LIVE(players[i])) send_despawn(players[i].sock, my_id);
        }

        pthread_mutex_unlock(&players_lock);

        close(new_socket);
        return NULL;
    }
}

void* new_conn(void* arg) {
    int server_fd = (int)(intptr_t)arg;
    while (true) {
        int new_socket = accept_client(server_fd);
        if (new_socket < 0) {
            continue;
        }

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_player, (void*)(intptr_t)new_socket);
        pthread_detach(client_thread);
    }
}

#endif
