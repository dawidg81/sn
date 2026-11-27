#ifndef PLUGIN_H
#define PLUGIN_H

lua_State *lua;
pthread_mutex_t lua_lock = PTHREAD_MUTEX_INITIALIZER;

void lua_player_join(const char *username){
    pthread_mutex_lock(&lua_lock);

    lua_getglobal(lua, "on_player_join");

    if (lua_isfunction(lua, -1)) {
        lua_pushstring(lua, username);

        if (lua_pcall(lua, 1, 0, 0) != LUA_OK) {
            fprintf(stderr, "Lua error: %s\n", lua_tostring(lua, -1));
            lua_pop(lua, 1);
        }
    } else {
        lua_pop(lua, 1);
    }

    pthread_mutex_unlock(&lua_lock);
}

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

#endif
