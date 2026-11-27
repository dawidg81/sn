#ifndef RELAY_H
#define RELAY_H

#include <stdarg.h>
#include "player.h"

void broadcast_message(const char* fmt, ...) {
    char msg[65];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    pthread_mutex_lock(&players_lock);
    for (int i = 0; i < 128; i++) {
        if (players[i].spawned && players[i].username != NULL) {
            send_message(players[i].sock, msg);
        }
    }
    pthread_mutex_unlock(&players_lock);
}

#endif
