#ifndef MAPGEN_H
#define MAPGEN_H

#include <stdint.h>

// Block types
typedef enum {
    BLOCK_AIR = 0,
    BLOCK_STONE = 1,
    BLOCK_GRASS = 2,
    BLOCK_DIRT = 3,
    BLOCK_COBBLESTONE = 4,
    BLOCK_OAK_WOOD = 5,
    BLOCK_OAK_LEAVES = 18,
    BLOCK_SAND = 12,
    BLOCK_GRAVEL = 13,
    BLOCK_GOLD_ORE = 14,
    BLOCK_IRON_ORE = 15,
    BLOCK_COAL_ORE = 16,
    BLOCK_LAVA = 11,
    BLOCK_WATER = 9,
    BLOCK_FLOWER_DANDELION = 37,
    BLOCK_FLOWER_ROSE = 38,
    BLOCK_MUSHROOM_BROWN = 39,
    BLOCK_MUSHROOM_RED = 40,
    BLOCK_BEDROCK = 7
} BlockType;

typedef struct {
    int width;
    int height;
    int depth;
    BlockType* blocks;
    int water_level;
} Level;

Level* mapgen_create_level(int width, int height, int depth, uint32_t seed);
void mapgen_free_level(Level* level);

static inline BlockType mapgen_get_block(Level* level, int x, int y, int z) {
    if (x < 0 || x >= level->width || y < 0 || y >= level->height || z < 0 || z >= level->depth)
        return BLOCK_AIR;
    return level->blocks[x + z * level->width + y * level->width * level->depth];
}

static inline void mapgen_set_block(Level* level, int x, int y, int z, BlockType block) {
    if (x < 0 || x >= level->width || y < 0 || y >= level->height || z < 0 || z >= level->depth)
        return;
    level->blocks[x + z * level->width + y * level->width * level->depth] = block;
}

#endif // MAPGEN_H
