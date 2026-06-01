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

#include "../inc/network_utils.h"
#include "../inc/socket.h"
#include "../inc/level.h"

#define	PORT	25565

int main()
{
	srand(time(NULL));

	int server_fd = setup_server_socket(PORT);	

	printf("Server ready\n");

	while (true) {
		int new_socket = accept_client(server_fd);

		// From here we handle client

		char buffer[131] = { 0 };
		valread = read(new_socket, buffer, sizeof(buffer) - 1);

		uint8_t packet_id = buffer[0];
		uint8_t protocol_version = buffer[1];
		char username[64] = { 0 };
		char verification_key[64] = { 0 };
		memcpy(username, buffer + 2, 63);
		memcpy(verification_key, buffer + 66, 63);
		uint8_t unused = buffer[130];

		printf("New client connected\n");
		printf("Their username is %s\n", username);
		printf("They are identifying with %s\n", verification_key);

		struct Level level;
		level.sizeX = 256;
		level.sizeY = 64;
		level.sizeZ = 256;

		int total = level.sizeX * level.sizeY * level.sizeZ;
		level.blocks = malloc(total);

		/*for(int i = 0; i < total; i++) {
			level.blocks[i] = rand() % 49;
		}*/

		sendLevel(new_socket, &level);
		free(level.blocks);
	}

	return 0;
}
