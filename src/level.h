#ifndef LEVEL_H
#define LEVEL_H

struct Level {
	short sizeX, sizeY, sizeZ;
	uint8_t* blocks;
};

struct Level level;

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
