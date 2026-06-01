#ifndef PLAYER_H
#define PLAYER_H

struct Player {
	char *username = "Player";
	uint8_t id = 0;

	float x = 0;
	float y = 0;
	float z = 0;
	float yaw = 0;
	float pitch = 0;
};

#endif
