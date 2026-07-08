#ifndef NOISE_H
#define NOISE_H

#include <math.h>
#include <stdint.h>

typedef struct {
    int permutation[512];
} PerlinNoise;

typedef struct {
    PerlinNoise* noises;
    int octaves;
} OctaveNoise;

typedef struct {
    OctaveNoise* noise1;
    OctaveNoise* noise2;
} CombinedNoise;

PerlinNoise* perlin_noise_create(uint32_t seed);
void perlin_noise_free(PerlinNoise* noise);
float perlin_noise_compute(PerlinNoise* noise, float x, float y);

OctaveNoise* octave_noise_create(uint32_t seed, int octaves);
void octave_noise_free(OctaveNoise* noise);
float octave_noise_compute(OctaveNoise* noise, float x, float y);

CombinedNoise* combined_noise_create(uint32_t seed1, uint32_t seed2, int octaves);
void combined_noise_free(CombinedNoise* noise);
float combined_noise_compute(CombinedNoise* noise, float x, float y);

#endif // NOISE_H
