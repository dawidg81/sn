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
#include "../inc/player.h"

#define	PORT	25565

int main() {
	srand(time(NULL));

	int server_fd = setup_server_socket(PORT);	

	printf("Server ready\n");

	while (true) {
		int new_socket = accept_client(server_fd);

		// From here we handle client
		read_id(new_socket);

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
