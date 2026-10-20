#ifndef CMDS_H
#define CMDS_H

#include "level.h"
#include "player.h"
#include "server.h"

struct {
  char *name;
  char *args[];
  void *func();
} Command;

void register_command(char *name_, char *args_[], void (*f)())
{
  Command cmd;
  cmd.name = name_;
  cmd.args = args_;
  cmd.func = f;
}

void handle_command(char *cmd)
{ /* game.h */
    if (strcmp(cmd, "save") == 0) {
        if (is_admin(me) != 0) {
            send_message(new_socket, "&eYou are not an op!");
            break;
        }
        save_level();
        send_message(new_socket, "&eWorld saved.");
        break;
    } else if (strcmp(cmd, "help") == 0) {
        save_level();
        send_message(new_socket, "&c/save &9- Save the world to file.");
        break;
    } else if (strcmp(cmd, "clients") == 0) {
        send_message(new_socket, "&cThe game does not handle the CEF plugin.");
        break;
    }
}

#endif
