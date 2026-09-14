#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>

#include "level.h"
#include "network_utils.h"
#include "player.h"
#include "server.h"
#include "socket.h"

#define PORT 25568

Player players[256];

int main() {
    srand(time(NULL));

    int server_fd = setup_server_socket(PORT);

    if (server_fd < 0) {
        return EXIT_FAILURE;
    }

    printf("Server ready\n");

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
            new_level(new_socket);
        }

        /*unsigned char b[100] = {0};
        ssize_t bsize = read(new_socket, b, sizeof(b));
        printf("%s\n", b);*/

        while (true) {
            unsigned char buf[1] = {0};
            ssize_t bufsize = read(new_socket, buf, sizeof(buf));
            if (bufsize <= 0) break;
            printf("DEBUG: Checking buffer\n");
            switch (buf[0]) {
                case 0x05:  // Set block
                    unsigned char packet[130] = {0};
                    ssize_t bytes = read(new_socket, packet, sizeof(packet));
                    if (bytes <= 0) break;

                    recv_block((char*)packet, &new_player);
                    break;
                case 0x08:  // Pos ort
                    unsigned char packet2[9] = {0};
                    ssize_t bytes2 = read(new_socket, packet2, sizeof(packet2));
                    if (bytes2 <= 0) break;

                    printf("DEBUG: Received packet 0x08\n");
                    fflush(stdout);
                    recv_pos_ort((char*)packet2, &new_player);
                    printf("DEBUG: Player pos: %f, %f, %f\n", new_player.x, new_player.y,
                           new_player.z);
                    fflush(stdout);
                    break;
                case 0x0d:
                    unsigned char packet3[65] = {0};
                    ssize_t bytes3 = read(new_socket, packet3, sizeof(packet3));
                    if (bytes3 <= 0) break;

                    printf("DEBUG: Received message packet from player: %s\n", packet3);
                    break;
                default:
                    printf("ERROR: player sent unknown packet %d\n", packet[0]);
                    break;
            }
        }
        close(new_socket);
    }

    return 0;
}
