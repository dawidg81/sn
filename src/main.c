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

#define	PORT	25568

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
		
		unsigned char buffer[1024] = { 0 };

		ssize_t valread = read(new_socket, buffer, sizeof(buffer));

		printf("new_socket = %d\n", new_socket);
		printf("read() = %zd\n", valread);

		if (valread <= 0) {
			perror("read");
			close(new_socket);
			continue;
		}

		// read_id(new_socket);
		new_level(new_socket);
		Player new_player;

		switch (buffer[0]) {
			case 0x00:
				if (init_player((char *)buffer, &new_player) != 0) {
					printf("Player initialization failed\n");
				}
				send_server_identification(new_socket, "A Minecraft Server", "Welcome!");
				break;
			case 0x05:
				recv_block(buffer, new_player);
				break;
		}
		close(new_socket);
	}

	return 0;
}
