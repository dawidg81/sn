#ifndef LEVEL_H
#define LEVEL_H

/**
 * @brief Defining an array of blocks.
 * @details
 * Includes X, Y, Z parameters defining size of
 * level. Makes unsigned byte integer as `blocks`
 * parameter.
 */
struct Level {
	short sizeX, sizeY, sizeZ;
	uint8_t* blocks;
};

/**
 * @brief A global `level` object of `Level` type.
 * @details
 * Gives a global, publicly accessible level object.
 * It's being created on start of the program.
 * It's the only operable level object so far.
 */
struct Level level;

/**
 * @brief Sends level to socket.
 * @details
 * Defines level data size by multiplicating level boundaries.
 * Allocates them in memory and adds them to `totalBlocks`.
 * Defines estimated size of compression to put in buffer.
 * Compresses whole array with gzip.
 *
 * After done compressing, sends Level Initialization Packet
 * with ID `0x02` to socket. Estimates total size of level
 * to send.
 *
 * It starts level serialization by sending chunks to socket in a
 * loop until it has done sending whole level. Each iteration is
 * chopping down the compressed level array into chunks made of
 * 1024 bytes plus 4 last bytes for offset confirmation, making
 * each buffer be 1028 bytes. That buffer is used to send
 * chunk level packet with ID `0x03`. It also includes byte
 * indicating percentage.
 *
 * If offset is less than total size of level to send, the loop
 * ends. To confirm that level sending completed, server sends to
 * client finalization packet with ID `0x04`. That packet gives
 * client information about level boundaries.
 */
void sendLevel(int socket, struct Level* level){
	int x = level->sizeX, y = level->sizeY, z = level->sizeZ;
	int totalBlocks = x * y * z;
	uint8_t* levelData = malloc(4 + totalBlocks);

	levelData[0] = (totalBlocks >> 24) & 0xFF;
	levelData[1] = (totalBlocks >> 16) & 0xFF;
	levelData[2] = (totalBlocks >> 8) & 0xFF;
	levelData[3] = totalBlocks & 0xFF;

	memcpy(levelData + 4, level->blocks, totalBlocks);

	uLongf compressedSize = compressBound(4 + totalBlocks) + 18;
	uint8_t* compressed = malloc(compressedSize);

	z_stream zs = {};
	int ret = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
	if(ret != Z_OK){ printf("Deflating failed: %d\n", ret); return; }

	zs.next_in = levelData;
	zs.avail_in = (uInt)(4 + totalBlocks);
	zs.next_out = compressed;
	zs.avail_out = (uInt)compressedSize;

	ret = deflate(&zs, Z_FINISH);
	if(ret != Z_STREAM_END){
		printf("Deflating did not finish: %d\n", ret);
		deflateEnd(&zs);
		return;
	}
	compressedSize = zs.total_out;
	deflateEnd(&zs);
	// compressed.resize(compressedSize);
	// logger.debug("Compr. size: " + to_string(compressedSize));
	// logger.debug("Chunks to send: " + to_string((compressedSize + 1023) / 1024));

	uint8_t initPacket = 0x02;
	send(socket, (char*)&initPacket, 1, 0);

	size_t offset = 0;
	size_t totalSize = compressedSize;

	while(offset < totalSize){
		char chunkPacket[1028] = {};
		size_t chunkLen = (1024 < (totalSize - offset)) ? 1024 : (totalSize - offset);
		uint8_t percent = (uint8_t)((offset + chunkLen) * 100 / totalSize);

		chunkPacket[0] = 0x03;
		chunkPacket[1] = (chunkLen >> 8) & 0xFF;
		chunkPacket[2] = chunkLen & 0xFF;
		memcpy(chunkPacket + 3, compressed + offset, chunkLen);
		chunkPacket[1027] = (char)percent;

		send(socket, chunkPacket, 1028, 0);
		offset += chunkLen;
	}

	uint8_t finalPacket[7];
	uint16_t sx = (uint16_t)x;
	uint16_t sy = (uint16_t)y;
	uint16_t sz = (uint16_t)z;

	finalPacket[0] = 0x04;
	write_u16_be(finalPacket, 1, x);
	write_u16_be(finalPacket, 3, y);
	write_u16_be(finalPacket, 5, z);
	send(socket, (char*)finalPacket, sizeof(finalPacket), 0);
	free(levelData);
	free(compressed);
}

/**
 * @brief Creates a new level.
 * @details
 * Creates a new level which is
 * 256 blocks wide and deep (X and Z axis)
 * and 64 blocks high (Y axis).
 *
 * Calculates total amount of blocks
 * from this level size and allocates them
 * into memory. After that, the level is instantly
 * sent to given socket then level blocks
 * are freed from memory as no longer used.
 *
 * Uses global `level` object.
 */
void new_level(int new_socket) {
	level.sizeX = 256;
	level.sizeY = 64;
	level.sizeZ = 256;

	int total = level.sizeX * level.sizeY * level.sizeZ;
	level.blocks = malloc(total);

	/*for(int i = 0; i < total; i++) {
	  level.blocks[i] = rand() % 49;
	  }*/

	sendLevel(new_socket, &level);
	free(level.blocks);
}

/**
 * @brief Set block on level.
 * @details
 * Lets you set a block with given `id` on
 * given `Level` type object coordinates
 * (`x`, `y`, `z`) in block data (not array!).
 * Checks if it tries to
 * set a block out of level boundaries.
 */
int level_set_block(struct Level* level, int x, int y, int z, uint8_t id) {
	if (
			x < 0 || x >= level->sizeX ||
			y < 0 || y >= level->sizeY ||
			z < 0 || z >= level->sizeZ
	   ) {
		printf("Tried to modify level out of bounds (%d, %d, %d)\n", x, y, z);
		return -1;
	}

	int index = y * (level->sizeX * level->sizeZ) + z * level->sizeX + x;

	level->blocks[index] = id;

	return 0;
}

/**
 */
uint8_t getBlock(struct Level* level, int x, int y, int z) {
	if (x < 0 || x >= level->sizeX ||
			y < 0 || y >= level->sizeY ||
			z < 0 || z >= level->sizeZ) {
		printf("Tried to check level block out of bounds (%d, %d, %d)\n", x, y, z);
		return 0; // air
	}

	int index = y * (level->sizeX * level->sizeZ) + z * level->sizeX + x;
	return level->blocks[index];
}

#endif
