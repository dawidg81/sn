#ifndef PLAYER_H
#define PLAYER_H

#include <errno.h>
#include "server.h"

int freeid = 0;

typedef struct {
	char *username;
	int8_t id;

	float x, y, z, yaw, pitch;
} Player;

int read_id(int new_socket, char *username, char *verification_key){
	char buffer[131] = { 0 };
	ssize_t valread = read(new_socket, buffer, sizeof(buffer) - 1);

	printf("DEBUG: new_socket = %d\n", new_socket);
	printf("DEBUG: valread = %zd, errno = %d\n", valread, errno);

	if (valread < 130) {
		perror("read");
		return -1;
	}

	uint8_t packet_id = buffer[0];
	uint8_t protocol_version = buffer[1];
	/*char username[64] = { 0 };
	  char verification_key[64] = { 0 };*/

	if (protocol_version != 0x10){
		send_disconnect(new_socket, "Invalid client version!");
		return -1;
	}

	memcpy(username, buffer + 2, 63);
	username[63] = '\0';

	memcpy(verification_key, buffer + 66, 63);
	verification_key[63] = '\0';

	uint8_t unused = buffer[130];

	printf("New client connected\n");
	printf("Their username is %s\n", username);
	printf("They are identifying with %s\n", verification_key);

	return 0;
}

int init_player(int new_socket) {
	Player new_player;

	char username[64] = { 0 };
	char verification_key[64] = { 0 };

	if (read_id(new_socket, username, verification_key) == 0) {
		new_player.username = malloc(64);
		strcpy(new_player.username, username);
	} else {
		return -1;
	}

	new_player.id = freeid++;

	new_player.x = new_player.y = new_player.z = 0;

	return 0;
}

#endif
