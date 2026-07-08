#include "mapgen.h"
#include "noise.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    int x, y, z;
} Vec3i;

typedef struct {
    Vec3i* items;
    int count;
    int capacity;
} Queue;

static Queue* queue_create(void) {
    Queue* q = malloc(sizeof(Queue));
    q->capacity = 1024;
    q->items = malloc(sizeof(Vec3i) * q->capacity);
    q->count = 0;
    return q;
}

static void queue_free(Queue* q) {
    free(q->items);
    free(q);
}

static void queue_push(Queue* q, int x, int y, int z) {
    if (q->count >= q->capacity) {
        q->capacity *= 2;
        q->items = realloc(q->items, sizeof(Vec3i) * q->capacity);
    }
    q->items[q->count].x = x;
    q->items[q->count].y = y;
    q->items[q->count].z = z;
    q->count++;
}

static Vec3i queue_pop(Queue* q) {
    return q->items[0];
}

static void queue_shift(Queue* q) {
    for (int i = 0; i < q->count - 1; i++) {
        q->items[i] = q->items[i + 1];
    }
    q->count--;
}

static void flood_fill(Level* level, int x, int y, int z, BlockType new_type, BlockType replace_type) {
    Queue* q = queue_create();
    queue_push(q, x, y, z);
    
    while (q->count > 0) {
        Vec3i pos = queue_pop(q);
        queue_shift(q);
        
        if (pos.x < 0 || pos.x >= level->width || 
            pos.y < 0 || pos.y >= level->height || 
            pos.z < 0 || pos.z >= level->depth) {
            continue;
        }
        
        if (mapgen_get_block(level, pos.x, pos.y, pos.z) != replace_type) {
            continue;
        }
        
        mapgen_set_block(level, pos.x, pos.y, pos.z, new_type);
        
        queue_push(q, pos.x + 1, pos.y, pos.z);
        queue_push(q, pos.x - 1, pos.y, pos.z);
        queue_push(q, pos.x, pos.y + 1, pos.z);
        queue_push(q, pos.x, pos.y - 1, pos.z);
        queue_push(q, pos.x, pos.y, pos.z + 1);
        queue_push(q, pos.x, pos.y, pos.z - 1);
    }
    
    queue_free(q);
}

static uint32_t rng_seed = 0;

static uint32_t rng_next(void) {
    rng_seed = (rng_seed * 1103515245 + 12345) & 0x7fffffff;
    return rng_seed;
}

static void rng_set_seed(uint32_t seed) {
    rng_seed = seed;
}

static float rng_float(void) {
    return (float)rng_next() / 0x7fffffff;
}

static int rng_int(int max) {
    return rng_next() % max;
}

static void fill_oblate_spheroid(Level* level, float cx, float cy, float cz, float radius) {
    int min_x = (int)(cx - radius);
    int max_x = (int)(cx + radius) + 1;
    int min_y = (int)(cy - radius);
    int max_y = (int)(cy + radius) + 1;
    int min_z = (int)(cz - radius);
    int max_z = (int)(cz + radius) + 1;
    
    for (int x = min_x; x <= max_x; x++) {
        for (int y = min_y; y <= max_y; y++) {
            for (int z = min_z; z <= max_z; z++) {
                float dx = x - cx;
                float dy = y - cy;
                float dz = z - cz;
                
                if ((dx*dx + 2*dy*dy + dz*dz) < radius*radius) {
                    if (x >= 0 && x < level->width && 
                        y >= 0 && y < level->height && 
                        z >= 0 && z < level->depth) {
                        if (mapgen_get_block(level, x, y, z) == BLOCK_STONE) {
                            mapgen_set_block(level, x, y, z, BLOCK_AIR);
                        }
                    }
                }
            }
        }
    }
}

static void generate_caves(Level* level, uint32_t seed) {
    rng_set_seed(seed);
    
    int total_caves = (level->width * level->height * level->depth) / 8192;
    
    for (int cave = 0; cave < total_caves; cave++) {
        float cave_x = rng_float() * level->width;
        float cave_y = rng_float() * level->height;
        float cave_z = rng_float() * level->depth;
        
        int cave_length = (int)((rng_float() + rng_float()) * 200);
        
        float theta = rng_float() * 3.14159f * 2.0f;
        float delta_theta = 0.0f;
        float phi = rng_float() * 3.14159f * 2.0f;
        float delta_phi = 0.0f;
        
        float cave_radius = rng_float() * rng_float();
        
        for (int len = 0; len < cave_length; len++) {
            cave_x += sinf(theta) * cosf(phi);
            cave_y += cosf(theta) * cosf(phi);
            cave_z += sinf(phi);
            
            delta_theta = delta_theta * 0.9f + (rng_float() - rng_float());
            theta += delta_theta * 0.2f;
            
            delta_phi = delta_phi * 0.75f + (rng_float() - rng_float());
            phi = phi / 2.0f + delta_phi / 4.0f;
            
            if (rng_float() >= 0.25f) {
                float center_x = cave_x + (rng_int(4) - 2) * 0.2f;
                float center_y = cave_y + (rng_int(4) - 2) * 0.2f;
                float center_z = cave_z + (rng_int(4) - 2) * 0.2f;
                
                float radius = (level->height - center_y) / (float)level->height;
                radius = 1.2f + (radius * 3.5f + 1.0f) * cave_radius;
                radius *= sinf(len * 3.14159f / cave_length);
                
                fill_oblate_spheroid(level, center_x, center_y, center_z, radius);
            }
        }
    }
}

static void generate_ore(Level* level, uint32_t seed, BlockType ore_type, float abundance) {
    rng_set_seed(seed);
    
    int total_veins = (level->width * level->height * level->depth * (int)(abundance * 100)) / 1638400;
    
    for (int vein = 0; vein < total_veins; vein++) {
        float vein_x = rng_float() * level->width;
        float vein_y = rng_float() * level->height;
        float vein_z = rng_float() * level->depth;
        
        int vein_length = (int)(rng_float() * rng_float() * 75.0f * abundance);
        
        float theta = rng_float() * 3.14159f * 2.0f;
        float delta_theta = 0.0f;
        float phi = rng_float() * 3.14159f * 2.0f;
        float delta_phi = 0.0f;
        
        for (int len = 0; len < vein_length; len++) {
            vein_x += sinf(theta) * cosf(phi);
            vein_y += cosf(theta) * cosf(phi);
            vein_z += sinf(phi);
            
            delta_theta = delta_theta * 0.9f + (rng_float() - rng_float());
            theta += delta_theta * 0.2f;
            
            delta_phi = delta_phi * 0.9f + (rng_float() - rng_float());
            phi = phi / 2.0f + delta_phi / 4.0f;
            
            float radius = abundance * sinf(len * 3.14159f / vein_length) + 1.0f;
            
            fill_oblate_spheroid(level, vein_x, vein_y, vein_z, radius);
            
            int min_x = (int)(vein_x - radius);
            int max_x = (int)(vein_x + radius) + 1;
            int min_y = (int)(vein_y - radius);
            int max_y = (int)(vein_y + radius) + 1;
            int min_z = (int)(vein_z - radius);
            int max_z = (int)(vein_z + radius) + 1;
            
            for (int x = min_x; x <= max_x; x++) {
                for (int y = min_y; y <= max_y; y++) {
                    for (int z = min_z; z <= max_z; z++) {
                        float dx = x - vein_x;
                        float dy = y - vein_y;
                        float dz = z - vein_z;
                        
                        if ((dx*dx + 2*dy*dy + dz*dz) < radius*radius) {
                            if (x >= 0 && x < level->width && 
                                y >= 0 && y < level->height && 
                                z >= 0 && z < level->depth) {
                                if (mapgen_get_block(level, x, y, z) == BLOCK_STONE) {
                                    mapgen_set_block(level, x, y, z, ore_type);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

static int is_space_for_tree(Level* level, int x, int z, int height) {
    int base_y = 0;
    for (int y = level->height - 1; y >= 0; y--) {
        if (mapgen_get_block(level, x, y, z) == BLOCK_GRASS) {
            base_y = y + 1;
            break;
        }
    }
    
    for (int tx = x - 1; tx <= x + 1; tx++) {
        for (int tz = z - 1; tz <= z + 1; tz++) {
            for (int ty = base_y; ty < base_y + height; ty++) {
                if (mapgen_get_block(level, tx, ty, tz) != BLOCK_AIR) {
                    return 0;
                }
            }
        }
    }
    
    return 1;
}

static void grow_tree(Level* level, int x, int y, int z, int height) {
    for (int ty = 0; ty < height - 3; ty++) {
        mapgen_set_block(level, x, y + ty, z, BLOCK_OAK_WOOD);
    }
    
    int canopy_start = height - 3;
    
    for (int tx = x - 2; tx <= x + 2; tx++) {
        for (int tz = z - 2; tz <= z + 2; tz++) {
            if (abs(tx - x) == 2 || abs(tz - z) == 2) {
                if (rng_float() < 0.5f) {
                    mapgen_set_block(level, tx, y + canopy_start, tz, BLOCK_OAK_LEAVES);
                }
            } else {
                mapgen_set_block(level, tx, y + canopy_start, tz, BLOCK_OAK_LEAVES);
            }
        }
    }
    mapgen_set_block(level, x, y + canopy_start, z, BLOCK_OAK_WOOD);
    
    for (int tx = x - 2; tx <= x + 2; tx++) {
        for (int tz = z - 2; tz <= z + 2; tz++) {
            if (abs(tx - x) != 2 && abs(tz - z) != 2) {
                mapgen_set_block(level, tx, y + canopy_start + 1, tz, BLOCK_OAK_LEAVES);
            }
        }
    }
    mapgen_set_block(level, x, y + canopy_start + 1, z, BLOCK_OAK_WOOD);
    
    for (int tx = x - 1; tx <= x + 1; tx++) {
        for (int tz = z - 1; tz <= z + 1; tz++) {
            mapgen_set_block(level, tx, y + canopy_start + 2, tz, BLOCK_OAK_LEAVES);
        }
    }
    mapgen_set_block(level, x, y + canopy_start + 2, z, BLOCK_OAK_WOOD);
    
    for (int tx = x - 1; tx <= x + 1; tx++) {
        for (int tz = z - 1; tz <= z + 1; tz++) {
            mapgen_set_block(level, tx, y + canopy_start + 3, tz, BLOCK_OAK_LEAVES);
        }
    }
    mapgen_set_block(level, x, y + canopy_start + 3, z, BLOCK_OAK_WOOD);
    
    mapgen_set_block(level, x, y - 1, z, BLOCK_DIRT);
}

Level* mapgen_create_level(int width, int height, int depth, uint32_t seed) {
    Level* level = (Level*)malloc(sizeof(Level));
    level->width = width;
    level->height = height;
    level->depth = depth;
    level->water_level = height / 2;
    
    int total_blocks = width * height * depth;
    level->blocks = (BlockType*)calloc(total_blocks, sizeof(BlockType));
    
    rng_set_seed(seed);
    
    int* height_map = (int*)malloc(width * depth * sizeof(int));
    
    CombinedNoise* noise1 = combined_noise_create(seed, seed + 1, 8);
    CombinedNoise* noise2 = combined_noise_create(seed + 2, seed + 3, 8);
    OctaveNoise* noise3 = octave_noise_create(seed + 4, 6);
    
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < depth; z++) {
            float height_low = combined_noise_compute(noise1, x * 1.3f, z * 1.3f) / 6.0f - 4.0f;
            float height_high = combined_noise_compute(noise2, x * 1.3f, z * 1.3f) / 5.0f + 6.0f;
            
            float height_result;
            if (octave_noise_compute(noise3, x, z) / 8.0f > 0) {
                height_result = height_low;
            } else {
                height_result = fmaxf(height_low, height_high);
            }
            
            height_result /= 2.0f;
            
            if (height_result < 0) {
                height_result *= 0.8f;
            }
            
            height_map[x + z * width] = (int)(height_result + level->water_level);
        }
    }
    
    CombinedNoise* smooth_noise1 = combined_noise_create(seed + 5, seed + 6, 8);
    CombinedNoise* smooth_noise2 = combined_noise_create(seed + 7, seed + 8, 8);
    
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < depth; z++) {
            float a = combined_noise_compute(smooth_noise1, x * 2.0f, z * 2.0f) / 8.0f;
            int b = combined_noise_compute(smooth_noise2, x * 2.0f, z * 2.0f) > 0 ? 1 : 0;
            
            if (a > 2) {
                height_map[x + z * width] = ((height_map[x + z * width] - b) / 2) * 2 + b;
            }
        }
    }
    
    combined_noise_free(noise1);
    combined_noise_free(noise2);
    combined_noise_free(smooth_noise1);
    combined_noise_free(smooth_noise2);
    octave_noise_free(noise3);
    
    OctaveNoise* strata_noise = octave_noise_create(seed + 9, 8);
    
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < depth; z++) {
            float dirt_thickness = octave_noise_compute(strata_noise, x, z) / 24.0f - 4.0f;
            int dirt_transition = height_map[x + z * width];
            int stone_transition = dirt_transition + (int)dirt_thickness;
            
            for (int y = 0; y < height; y++) {
                BlockType block_type = BLOCK_AIR;
                
                if (y == 0) {
                    block_type = BLOCK_LAVA;
                } else if (y <= stone_transition) {
                    block_type = BLOCK_STONE;
                } else if (y <= dirt_transition) {
                    block_type = BLOCK_DIRT;
                }
                
                mapgen_set_block(level, x, y, z, block_type);
            }
        }
    }
    
    octave_noise_free(strata_noise);
    
    generate_caves(level, seed + 10);
    
    generate_ore(level, seed + 11, BLOCK_COAL_ORE, 0.9f);
    generate_ore(level, seed + 12, BLOCK_IRON_ORE, 0.7f);
    generate_ore(level, seed + 13, BLOCK_GOLD_ORE, 0.5f);
    
    // Fill from edges
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < depth; z++) {
            if (mapgen_get_block(level, x, level->water_level - 1, z) == BLOCK_AIR) {
                flood_fill(level, x, level->water_level - 1, z, BLOCK_WATER, BLOCK_AIR);
            }
        }
    }
    
    int water_sources = width * depth / 8000;
    for (int i = 0; i < water_sources; i++) {
        int x = rng_int(width);
        int z = rng_int(depth);
        int y = rng_float() < 0.5f ? level->water_level - 1 : level->water_level - 2;
        
        if (mapgen_get_block(level, x, y, z) == BLOCK_AIR) {
            flood_fill(level, x, y, z, BLOCK_WATER, BLOCK_AIR);
        }
    }
    
    int lava_sources = width * depth / 20000;
    for (int i = 0; i < lava_sources; i++) {
        int x = rng_int(width);
        int z = rng_int(depth);
        int y = (int)((level->water_level - 3) * rng_float() * rng_float());
        
        if (y > 0 && mapgen_get_block(level, x, y, z) == BLOCK_AIR) {
            flood_fill(level, x, y, z, BLOCK_LAVA, BLOCK_AIR);
        }
    }
    
    OctaveNoise* surface_noise1 = octave_noise_create(seed + 14, 8);
    OctaveNoise* surface_noise2 = octave_noise_create(seed + 15, 8);
    
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < depth; z++) {
            int sand_chance = octave_noise_compute(surface_noise1, x, z) > 8.0f ? 1 : 0;
            int gravel_chance = octave_noise_compute(surface_noise2, x, z) > 12.0f ? 1 : 0;
            
            int y = height_map[x + z * width];
            BlockType block_above = mapgen_get_block(level, x, y + 1, z);
            
            if (block_above == BLOCK_WATER && gravel_chance) {
                mapgen_set_block(level, x, y, z, BLOCK_GRAVEL);
            } else if (block_above == BLOCK_AIR) {
                if (y <= level->water_level && sand_chance) {
                    mapgen_set_block(level, x, y, z, BLOCK_SAND);
                } else {
                    mapgen_set_block(level, x, y, z, BLOCK_GRASS);
                }
            }
        }
    }
    
    octave_noise_free(surface_noise1);
    octave_noise_free(surface_noise2);
    
    // Flowers
    int flower_patches = width * depth / 3000;
    for (int patch = 0; patch < flower_patches; patch++) {
        BlockType flower_type = rng_int(2) == 0 ? BLOCK_FLOWER_DANDELION : BLOCK_FLOWER_ROSE;
        int patch_x = rng_int(width);
        int patch_z = rng_int(depth);
        
        for (int i = 0; i < 10; i++) {
            int flower_x = patch_x;
            int flower_z = patch_z;
            
            for (int j = 0; j < 5; j++) {
                flower_x += rng_int(6) - rng_int(6);
                flower_z += rng_int(6) - rng_int(6);
                
                if (flower_x >= 0 && flower_x < width && flower_z >= 0 && flower_z < depth) {
                    int flower_y = height_map[flower_x + flower_z * width] + 1;
                    BlockType block_below = mapgen_get_block(level, flower_x, flower_y - 1, flower_z);
                    
                    if (mapgen_get_block(level, flower_x, flower_y, flower_z) == BLOCK_AIR && 
                        block_below == BLOCK_GRASS) {
                        mapgen_set_block(level, flower_x, flower_y, flower_z, flower_type);
                    }
                }
            }
        }
    }
    
    // Mushrooms
    int mushroom_patches = width * height * depth / 2000;
    for (int patch = 0; patch < mushroom_patches; patch++) {
        BlockType mushroom_type = rng_int(2) == 0 ? BLOCK_MUSHROOM_BROWN : BLOCK_MUSHROOM_RED;
        int patch_x = rng_int(width);
        int patch_y = rng_int(height);
        int patch_z = rng_int(depth);
        
        for (int i = 0; i < 20; i++) {
            int mush_x = patch_x;
            int mush_y = patch_y;
            int mush_z = patch_z;
            
            for (int j = 0; j < 5; j++) {
                mush_x += rng_int(6) - rng_int(6);
                mush_y += rng_int(2) - rng_int(2);
                mush_z += rng_int(6) - rng_int(6);
                
                if (mush_x >= 0 && mush_x < width && mush_y >= 0 && mush_y < height && 
                    mush_z >= 0 && mush_z < depth && mush_y < height_map[mush_x + mush_z * width] - 1) {
                    BlockType block_below = mapgen_get_block(level, mush_x, mush_y - 1, mush_z);
                    
                    if (mapgen_get_block(level, mush_x, mush_y, mush_z) == BLOCK_AIR && 
                        block_below == BLOCK_STONE) {
                        mapgen_set_block(level, mush_x, mush_y, mush_z, mushroom_type);
                    }
                }
            }
        }
    }
    
    // Trees
    int tree_patches = width * depth / 4000;
    for (int patch = 0; patch < tree_patches; patch++) {
        int patch_x = rng_int(width);
        int patch_z = rng_int(depth);
        
        for (int i = 0; i < 20; i++) {
            int tree_x = patch_x;
            int tree_z = patch_z;
            
            for (int j = 0; j < 20; j++) {
                tree_x += rng_int(6) - rng_int(6);
                tree_z += rng_int(6) - rng_int(6);
                
                if (tree_x >= 0 && tree_x < width && tree_z >= 0 && tree_z < depth && 
                    rng_float() <= 0.25f) {
                    int tree_y = height_map[tree_x + tree_z * width] + 1;
                    int tree_height = rng_int(3) + 4;
                    
                    if (is_space_for_tree(level, tree_x, tree_z, tree_height)) {
                        grow_tree(level, tree_x, tree_y, tree_z, tree_height);
                    }
                }
            }
        }
    }
    
    free(height_map);
    
    return level;
}

void mapgen_free_level(Level* level) {
    free(level->blocks);
    free(level);
}
