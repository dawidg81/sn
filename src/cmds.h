#ifndef CMDS_H
#define CMDS_H

#include "level.h"
#include "player.h"
#include "server.h"

void handle_command(char *cmd, Player *you)
{
    printf("DEBUG: %s used command: %s", you->username, cmd);
    if (strcmp(cmd, "save") == 0) {
        if (is_admin(you) != 0) {
            send_message(you->sock, "&eYou are not an op!");
            return;
        }
        save_level();
        send_message(you->sock, "&eWorld saved.");
        return;
    } else if (strcmp(cmd, "help") == 0) {
        save_level();
        send_message(you->sock, "&c/save &9- Save the world to file.");
        return;
    } else if (strcmp(cmd, "clients") == 0) {
        send_message(you->sock, "&cThe game does not handle the CEF plugin.");
        return;
    }
}

#endif
