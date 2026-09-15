#ifndef MAPGEN_H
#define MAPGEN_H

#include "level.h"

#define STONE_ID 1
#define DIRT_ID 2
#define GRASS_ID 3
#define WATER_ID 9

void flatgen_generate(struct Level* level) {
    int ground_level = 32;
    int total = level->sizeX * level->sizeY * level->sizeZ;

    for (int i = 0; i < total; i++) {
        level->blocks[i] = 0;
    }

    for (int x = 0; x < level->sizeX; x++) {
        for (int z = 0; z < level->sizeZ; z++) {
            for (int y = 0; y < ground_level; y++) {
                level_set_block(level, x, y, z, STONE_ID);
            }
            if (ground_level < level->sizeY) {
                level_set_block(level, x, ground_level, z, GRASS_ID);
            }
        }
    }
}

#endif
