#ifndef SERVER_H
#define SERVER_H

void send_server_identification(int socket, char name[], char motd[]) {
	char buffer[131] = { 0 };
	uint8_t pid = 0x00;
	uint8_t prot_ver = 0x07;
	uint8_t utype = 0x64; // player is admin by default for now

	buffer[0] = pid;
	buffer[1] = prot_ver;
	write_str64((uint8_t *)buffer, 2, name);
	write_str64((uint8_t *)buffer, 66, motd);
	buffer[130] = utype;
}

void send_disconnect(int socket, char message[]) {
	/*
	 * Packet ID - a byte
	 * Message - string, 64 bytes
	 * 	(null termination at end)
	 * 65 bytes for buffer in total
	 */

	char buffer[65] = { 0 };
	uint8_t pid = 0x0e;
	
	buffer[0] = pid;
	write_str64((uint8_t *)buffer, 1, message);

	send(socket, buffer, sizeof(buffer), 0);
}

#endif
