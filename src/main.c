#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>

#include "game.h"
#include "heartbeat.h"
#include "level.h"
#include "network_utils.h"
#include "player.h"
#include "server.h"
#include "socket.h"

#define PORT 25568

int main() {
    srand(time(NULL));
    signal(SIGPIPE, SIG_IGN);
    level_init();

    int server_fd = setup_server_socket(PORT);

    if (server_fd < 0) {
        return EXIT_FAILURE;
    }

    heartbeat_start(PORT, "the bedrock lounge [chat with others]", 128);

    printf("Server ready\n");

    pthread_t gameloop;
    pthread_create(&gameloop, NULL, new_conn, (void*)(intptr_t)server_fd);
    // handle_player(server_fd);

    pthread_join(gameloop, NULL);

    return 0;
}
