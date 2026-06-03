#ifndef SERVER_H
#define SERVER_H

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
