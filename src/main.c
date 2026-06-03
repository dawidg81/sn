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

int main() {
	srand(time(NULL));

	int server_fd = setup_server_socket(PORT);

	printf("Server ready\n");

	while (true) {
		int new_socket = accept_client(server_fd);

		// From here we handle client
		init_player(new_socket);
		new_level(new_socket);
	}

	return 0;
}
