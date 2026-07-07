#ifndef PLAYER_H
#define PLAYER_H

#include <errno.h>
#include "server.h"
#include "level.h"
#include "network_utils.h"

/**
 * @brief Determines next free player ID.
 * @details
 * This variable stores next ID number that
 * should be free for next player that joins.
 * It should increment each new connection.
 */
int freeid = 0;

/**
 * @brief Defines player.
 * @details
 * Defines player instance and its parameters.
 */
typedef struct {
	/** Player username string, array of characters. */
	char *username;
	/** Internal ID of player to operate on it.*/
	int8_t id;

	/** X coordinate of player position. */
	float x;
	/** Y coordinate of player position. */
	float y;
	/** Z coordinate of player position. */
	float z;
	/** Yaw angle of player's orientation. */
	float yaw;
	/** Pitch angle of player's orientation. */
	float pitch;
} Player;

int read_id(char *buffer, char *username, char *verification_key){
	// char buffer[131] = { 0 };
	// ssize_t valread = read(new_socket, buffer, sizeof(buffer));

	/*printf("DEBUG: new_socket = %d\n", new_socket);
	printf("DEBUG: valread = %zd, errno = %d\n", valread, errno);*/

	/*if (valread < 131) {
		perror("read");
		return -1;
	}*/

	// uint8_t packet_id = buffer[0];

	uint8_t protocol_version = buffer[1];
	/*char username[64] = { 0 };
	  char verification_key[64] = { 0 };*/

	/*if (protocol_version != 0x07){
		send_disconnect(new_socket, "Invalid client version!");
		return -1;
	}*/

	memcpy(username, buffer + 2, 64);
	username[64] = '\0';

	memcpy(verification_key, buffer + 66, 64);
	verification_key[64] = '\0';

	// uint8_t unused = buffer[129];

	printf("New client connected\n");
	printf("Their username is %s\n", username);
	printf("They are identifying with %s\n", verification_key);

	return 0;
}

int init_player(char *buffer) {
	Player new_player;

	char username[64] = { 0 };
	char verification_key[64] = { 0 };

	if (read_id(buffer, username, verification_key) == 0) {
		new_player.username = malloc(64);
		strcpy(new_player.username, username);
	} else {
		return -1;
	}

	new_player.id = freeid++;

	new_player.x = new_player.y = new_player.z = 0;

	return 0;
}

void recv_block(char *buffer, Player new_player) {
	// char buffer[8] = { 0 };
	// ssize_t valread = read(socket, buffer, sizeof(buffer));

	// uint8_t pid = buffer[0];

	/*if (pid != 0x05) {
		printf("Did not receive proper packet ID for Set Block packet (expected 0x05 but got %d)\n", pid);
	}*/

	uint16_t x = read_u16_be((uint8_t*)buffer, buffer[0]);
	uint16_t y = read_u16_be((uint8_t*)buffer, buffer[2]);
	uint16_t z = read_u16_be((uint8_t*)buffer, buffer[4]);

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
