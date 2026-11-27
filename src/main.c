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

lua_State *lua;
pthread_mutex_t lua_lock = PTHREAD_MUTEX_INITIALIZER;

void safe_exit(){
    save_level();
    exit(0);
    return;
}

void lua_player_join();

int lua_say(lua_State *L){
    const char *message = luaL_checkstring(L, 1);
    broadcast_message("%s", message);
    return 0;
}

void lua_init(void){
    lua = luaL_newstate();

    if (!lua) {
        fprintf(stderr, "ERROR: Failed to initialize Lua!\n");
        exit(EXIT_FAILURE);
    }

    luaL_openlibs(lua);
    lua_register(lua, "say", lua_say);

    if (luaL_dofile(lua, "plugins.lua") != LUA_OK) {
        fprintf(stderr, "ERROR: Lua error: %s\n", lua_tostring(lua, -1));
        lua_pop(lua, 1);
    }
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
