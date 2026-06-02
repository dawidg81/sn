#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <zlib.h>
#include <time.h>

#include "network_utils.h"
#include "socket.h"
#include "level.h"
#include "player.h"

#define	PORT	25565

int freeid = 0;

int main() {
	srand(time(NULL));

	int server_fd = setup_server_socket(PORT);

	printf("Server ready\n");

	while (true) {
		int new_socket = accept_client(server_fd);

		// From here we handle client
		Player new_player;

        char username[64] = { 0 };
        char verification_key[64] = { 0 };

        if (read_id(new_socket, username, verification_key) == 0) {
            new_player.username = malloc(64);
            strcpy(new_player.username, username);
        } else {
            continue;
        }

		new_player.id = freeid++;

		new_player.x = new_player.y = new_player.z = 0;

		new_level(new_socket);
	}

	return 0;
}
