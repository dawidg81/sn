#ifndef CMDS_H
#define CMDS_H

#include "level.h"
#include "player.h"
#include "server.h"

void handle_command(char *cmd, Player *you, Player **players)
{
    printf("DEBUG: %s used command: %s\n", you->username, cmd);
    if (strcmp(cmd, "save") == 0) {
        if (is_admin(you) != 0) {
            send_message(you->sock, "&eYou are not an op!");
            return;
        }
        save_level();
        send_message(you->sock, "&eWorld saved.");
        return;
    } else if (strcmp(cmd, "help") == 0) {
        send_message(you->sock, "&c/save &9- Save the world to file.");
        return;
    } else if (strcmp(cmd, "clients") == 0) {
        send_message(you->sock, "&cThe game does not handle the CEF plugin.");
        return;
    } else if (strncmp(cmd, "kick ", 5) == 0) {
        /*char *args = strtok(cmd, " ");
        args = strtok(NULL, " ");*/
        // printf("DEBUG: kick arg: %s\n", args);
        for (int i=0; i < 128; i++) {
            if (players[i]->username == NULL || !players[i]->spawned) continue;
            printf("DEBUG: cmds+5 = %s\n", cmd + 5);
            printf("DEBUG: name = %s\n", players[i]->username);
            if (strcmp(players[i]->username, cmd + 5) == 0) {
                send_disconnect(players[i]->sock, "You have been kicked");
                free(players[i]->username);
                free(players[i]->verification_key);
                close(players[i]->sock);
                break;
            }
        }
    }
}

#endif
