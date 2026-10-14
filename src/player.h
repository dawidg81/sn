#ifndef PLAYER_H
#define PLAYER_H

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "level.h"
#include "network_utils.h"
#include "server.h"

int freeid = 0;

typedef struct {
    char* username;
    char* verification_key;
    int8_t id;
    int sock;
    int spawned;  /* should be 1 once level is sent and player is visible to others */
    float x, y, z, yaw, pitch;
} Player;

int read_id(char* buffer, char* username, char* verification_key) {
    uint8_t protocol_version = buffer[1];

    read_str64((uint8_t*)buffer, 2, username);
    read_str64((uint8_t*)buffer, 66, verification_key);

    printf("DEBUG: New client connected\n");
    printf("DEBUG: Their username is %s\n", username);
    printf("DEBUG: They are identifying with %s\n", verification_key);
    printf("DEBUG: Gave them ID number %d\n", freeid);

    return 0;
}

int init_player(char* buffer, Player* player) {
    char username[65] = {0};
    char verification_key[65] = {0};
    /*username[64] = '\0';
    verification_key[64] = '\0';*/

    if (read_id(buffer, username, verification_key) == 0) {
        player->username = malloc(65);
        strcpy(player->username, username);
        player->verification_key = malloc(65);
        strcpy(player->verification_key, verification_key);
    } else {
        return -1;
    }
	
	player->id = freeid++;

    player->x = player->y = player->z = 0;

    return 0;
}

/*void recv_block(char* buffer, Player* new_player) {
    uint16_t x = read_u16_be((const uint8_t*)buffer, 0);
    uint16_t y = read_u16_be((const uint8_t*)buffer, 2);
    uint16_t z = read_u16_be((const uint8_t*)buffer, 4);

    uint8_t mode = buffer[6];
    uint8_t block_id = buffer[7];

    if (mode == 0x01) {
        level_set_block(&level, x, y, z, block_id);
    } else {
        level_set_block(&level, x, y, z, 0);
    }
}*/

int recv_block(char* buffer, uint16_t* x, uint16_t* y, uint16_t* z, uint8_t* block) {
    *x = read_u16_be((const uint8_t*)buffer, 0);
    *y = read_u16_be((const uint8_t*)buffer, 2);
    *z = read_u16_be((const uint8_t*)buffer, 4);
    uint8_t mode = buffer[6];
    *block = (mode == 0x01) ? buffer[7] : 0;

    if (*x >= level.sizeX || *y >= level.sizeY || *z >= level.sizeZ) return -1;
    level_set_block(&level, *x, *y, *z, *block);
    return 0;
}

void recv_pos_ort(char* buffer, Player* player) {
    /* ssize_t valread = read(socket, buffer, sizeof(buffer)); */

    uint8_t player_id = buffer[0];

    int16_t x_fixed = (int16_t)read_u16_be((const uint8_t*)buffer, 1);
    int16_t y_fixed = (int16_t)read_u16_be((const uint8_t*)buffer, 3);
    int16_t z_fixed = (int16_t)read_u16_be((const uint8_t*)buffer, 5);
    uint8_t yaw = buffer[7];
    uint8_t pitch = buffer[8];

    player->x = x_fixed / 32.0f;
    player->y = y_fixed / 32.0f;
    player->z = z_fixed / 32.0f;
    player->yaw = yaw;
    player->pitch = pitch;
}

void recv_message(char* buffer, Player* player, char* out_message) {
    uint8_t player_id = buffer[0];
    read_str64((uint8_t*)buffer, 1, out_message);
}

/* FILE I/O */

int is_whitelisted(Player* player)
{
    /* 0 = whitelisted; 1 = not whitelisted */
    FILE* file = fopen("whitelist.txt", "r");
    if(file == NULL){
        printf("Failed to open whitelist.txt! Does it exist?\n");
        fclose(file);
        return 1;
    }

    char line[65];
    while(fgets(line, sizeof(line), file) != NULL){
        line[strcspn(line, "\r\n")] = '\0';
        if(strcmp(line, player->username) == 0){fclose(file);return 0;}
    }

    fclose(file);
    return 1;
}

#endif
