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

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "game.h"
#include "heartbeat.h"
#include "level.h"
#include "network_utils.h"
#include "player.h"
#include "server.h"
#include "socket.h"
#include "attr.h"
#include "plugin.h"

void safe_exit(){
    save_level();
    exit(0);
    return;
}

int main() {
    srand(time(NULL));
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, safe_exit);

    config_load("config.txt");
    level_init();
    
    int r = load_level();
    if(r == 1){
        printf("\bWARNING: Failed to load world data from file. The world will be empty!\n");
    } else if(r == -1){
        printf("There is no world file saved so far. The world will be empty.\n");
    }

    int server_fd = setup_server_socket(server_port);

    if (server_fd < 0) {
        return EXIT_FAILURE;
    }

    heartbeat_start(server_port, server_name, 128);
    lua_init();

    printf("Server ready\n");

    pthread_t gameloop;
    pthread_create(&gameloop, NULL, new_conn, (void*)(intptr_t)server_fd);
    // handle_player(server_fd);

    pthread_join(gameloop, NULL);

    return 0;
}
