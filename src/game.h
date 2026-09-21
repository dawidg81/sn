#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "socket.h"
#include "player.h"
#include "server.h"
#include "level.h"

Player players[256];

void handle_player(void *arg)
{
    int server_fd = *(int*)arg;

    while (true) {
            int new_socket = accept_client(server_fd);

            if (new_socket < 0) {
                continue;
            }

            // From here we handle client

            unsigned char buffer[131] = {0};

            ssize_t valread = read(new_socket, buffer, sizeof(buffer));

            if (valread <= 0) {
                perror("read");
                close(new_socket);
                continue;
            }

            // read_id(new_socket);

            Player new_player;

            if (buffer[0] == 0x00) {
                if (init_player((char*)buffer, &new_player) != 0) {
                    printf("Player initialization failed");
                    close(new_socket);
                    continue;
                }
                send_server_identification(new_socket, "A Minecraft Server", "Welcome!");
                players[new_player.id] = new_player;  // appending player to global table
                new_level(new_socket);
            }

            /*unsigned char b[100] = {0};
            ssize_t bsize = read(new_socket, b, sizeof(b));
            printf("%s\n", b);*/

            send_spawn(new_socket, -1, new_player.username, level.sizeX / 2, level.sizeY,
                       level.sizeZ / 2, 0x00, 0x00);
            send_message(new_socket, "a potatoe e tomatoe");

            while (true) {
                unsigned char buf[1] = {0};
                ssize_t bufsize = read(new_socket, buf, sizeof(buf));
                if (bufsize <= 0) break;
                switch (buf[0]) {
                    case 0x05: { // Set block
                        unsigned char packet[8] = {0};
                        ssize_t bytes = read(new_socket, packet, sizeof(packet));
                        if (bytes <= 0) break;

                        recv_block((char*)packet, &new_player);
                    }   break;
                    case 0x08: { // Pos ort
                        unsigned char packet[9] = {0};
                        ssize_t bytes = read(new_socket, packet, sizeof(packet));
                        if (bytes <= 0) break;

                        fflush(stdout);
                        recv_pos_ort((char*)packet, &new_player);
                        fflush(stdout);
                    }   break;
                    case 0x0d: { // Message
                        unsigned char packet[65] = {0};
                        ssize_t bytes = read(new_socket, packet, sizeof(packet));
                        if (bytes <= 0) break;

                        char received[64] = {0};
                        recv_message((char*)packet, &new_player, received);

                        char msg[64];
                        snprintf(msg, sizeof(msg), "%s: %s", new_player.username, received);

                        send_message(new_socket, msg);

                    }   break;
                    default:
                        printf("ERROR: player sent unknown packet %d\n", buf[0]);
                        break;
                }
            }
            close(new_socket);
        }
}

#endif
