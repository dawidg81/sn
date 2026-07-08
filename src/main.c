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
#include "server.h"

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

		while(true){
			unsigned char packet[1024] = { 0 };
			ssize_t bytes = read(new_socket, packet, sizeof(packet));
			if (bytes <= 0) break;
			switch (packet[0]) {
				case 0x05: // Set block
					recv_block((char *)packet, new_player);
					break;
			}
		}
		close(new_socket);
	}

	return 0;
}
