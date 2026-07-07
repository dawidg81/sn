#ifndef PLAYER_H
#define PLAYER_H

#include <errno.h>
#include "server.h"
#include "level.h"
#include "network_utils.h"

int freeid = 0;

typedef struct {
	char *username;
	int8_t id;

	float x, y, z, yaw, pitch;
} Player;

int read_id(char *buffer, char *username, char *verification_key){
	uint8_t protocol_version = buffer[1];

	memcpy(username, buffer + 2, 64);
	username[64] = '\0';

	memcpy(verification_key, buffer + 66, 64);
	verification_key[64] = '\0';

	printf("New client connected\n");
	printf("Their username is %s\n", username);
	printf("They are identifying with %s\n", verification_key);

	return 0;
}

int init_player(char *buffer, Player *player) {
	char username[65] = { 0 };
	char verification_key[65] = { 0 };
	username[64] = '\0';
	username[64] = '\0';

	if (read_id(buffer, username, verification_key) == 0) {
		player->username = malloc(65);
		strcpy(player->username, username);
	} else {
		return -1;
	}

	player->id = freeid++;

	player->x = player->y = player->z = 0;

	return 0;
}

void recv_block(char *buffer, Player new_player) {
	uint16_t x = read_u16_be((const uint8_t *)buffer, 1);
	uint16_t y = read_u16_be((const uint8_t *)buffer, 3);
	uint16_t z = read_u16_be((const uint8_t *)buffer, 5);
	
	uint8_t mode = buffer[6];
	uint8_t block_id = buffer[7];

	if (mode == 0x01) {
		level_set_block(&level, x, y, z, block_id);
	} else {
		level_set_block(&level, x, y, z, 0); // player break -> air
	}
}

/*void recv_pos_ort(int socket, Player player) {
	char buffer[9] = { 0 };
	ssize_t valread = read(socket, buffer, sizeof(buffer));

	uint8_t pid = buffer[0];

	if (pid != 0x08) {
		printf("Did not receive proper packet ID for Set Block packet (expected 0x05 but got %d)\n", pid);
	}

	uint16_t x = read_u16_be((uint8_t)buffer, buffer[1]);
	uint16_t y = read_u16_be((uint8_t)buffer, buffer[3]);
	uint16_t z = read_u16_be((uint8_t)buffer, buffer[5]);
	uint8_t mode = buffer[7];
	uint8_t block_id = buffer[8];

	if (mode == 0x01) {
		level_set_block(level, x, y, z, id);
	} else {
		level_set_block(level, x, y, z, 0); // player break -> air
	}
}*/

#endif
