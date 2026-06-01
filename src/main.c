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

	int server_fd, new_socket;
	ssize_t valread;
	struct sockaddr_in address;
	int opt = 1;
	socklen_t addrlen = sizeof(address);

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	printf("Server ready\n");

	while (true) {
		if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

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

		for(int i = 0; i < total; i++) {
			level.blocks[i] = rand() % 49;
		}

		// memset(level.blocks, rand() % 65, total);
		sendLevel(new_socket, &level);
		free(level.blocks);
	}

	return 0;
}
