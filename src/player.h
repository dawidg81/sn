#ifndef PLAYER_H
#define PLAYER_H

struct Player {
	char *username;
	uint8_t id;

	float x, y, z, yaw, pitch;
};

void read_id(int new_socket){
	char buffer[131] = { 0 };
	ssize_t valread = read(new_socket, buffer, sizeof(buffer) - 1);

	uint8_t packet_id = buffer[0];
	uint8_t protocol_version = buffer[1];
	char username[64] = { 0 };
	char verification_key[64] = { 0 };
	memcpy(username, buffer + 2, 63);
	memcpy(verification_key, buffer + 66, 63);
	uint8_t unused = buffer[130];

	printf("New client connected\n");
	printf("Their username is %s\n", username);
	printf("They are identifying with %s\n", verification_key);
}

#endif
