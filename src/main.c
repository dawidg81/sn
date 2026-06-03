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

#define	PORT	25567

Player players[256];

int main() {
	srand(time(NULL));

	int server_fd = setup_server_socket(PORT);

	printf("Server ready\n");

	while (true) {
		int new_socket = accept_client(server_fd);

		// From here we handle client
		
		unsigned char buffer[1024] = { 0 };

		ssize_t valread = read(new_socket, buffer, sizeof(buffer));

		// read_id(new_socket);
		new_level(new_socket);

		switch (buffer[0]) {
			case 0x00:
				if (init_player(buffer) != 0) {
					printf("Player initialization failed\n");
				}
			case 0x05:
				recv_block(buffer, new_player);
		}
	}

	return 0;
}
