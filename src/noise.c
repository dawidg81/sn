#include "noise.h"
#include <stdlib.h>
#include <string.h>

static uint32_t lcg_next(uint32_t* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

static float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static float grad(int hash, float x, float y) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 8 ? y : x;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

PerlinNoise* perlin_noise_create(uint32_t seed) {
    PerlinNoise* noise = (PerlinNoise*)malloc(sizeof(PerlinNoise));
    
    for (int i = 0; i < 256; i++) {
        noise->permutation[i] = i;
    }
    
    uint32_t rng_seed = seed;
    for (int i = 255; i > 0; i--) {
        int j = lcg_next(&rng_seed) % (i + 1);
        int temp = noise->permutation[i];
        noise->permutation[i] = noise->permutation[j];
        noise->permutation[j] = temp;
    }
    
    for (int i = 0; i < 256; i++) {
        noise->permutation[256 + i] = noise->permutation[i];
    }
    
    return noise;
}

void perlin_noise_free(PerlinNoise* noise) {
    free(noise);
}

float perlin_noise_compute(PerlinNoise* noise, float x, float y) {
    int xi = (int)floor(x) & 255;
    int yi = (int)floor(y) & 255;
    
    float xf = x - floor(x);
    float yf = y - floor(y);
    
    float u = fade(xf);
    float v = fade(yf);
    
    int p0 = noise->permutation[xi];
    int p1 = noise->permutation[xi + 1];
    
    int aa = noise->permutation[p0 + yi];
    int ab = noise->permutation[p0 + yi + 1];
    int ba = noise->permutation[p1 + yi];
    int bb = noise->permutation[p1 + yi + 1];
    
    float g0 = grad(noise->permutation[aa], xf, yf);
    float g1 = grad(noise->permutation[ba], xf - 1, yf);
    float g2 = grad(noise->permutation[ab], xf, yf - 1);
    float g3 = grad(noise->permutation[bb], xf - 1, yf - 1);
    
    float lerp1 = lerp(g0, g1, u);
    float lerp2 = lerp(g2, g3, u);
    
    return lerp(lerp1, lerp2, v);
}

OctaveNoise* octave_noise_create(uint32_t seed, int octaves) {
    OctaveNoise* noise = (OctaveNoise*)malloc(sizeof(OctaveNoise));
    noise->octaves = octaves;
    noise->noises = (PerlinNoise*)malloc(sizeof(PerlinNoise*) * octaves);
    
    for (int i = 0; i < octaves; i++) {
        noise->noises[i] = *perlin_noise_create(seed + i);
    }
    
    return noise;
}

void octave_noise_free(OctaveNoise* noise) {
    for (int i = 0; i < noise->octaves; i++) {
        perlin_noise_free(&noise->noises[i]);
    }
    free(noise->noises);
    free(noise);
}

float octave_noise_compute(OctaveNoise* noise, float x, float y) {
    float result = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float max_amplitude = 0.0f;
    
    for (int i = 0; i < noise->octaves; i++) {
        result += perlin_noise_compute(&noise->noises[i], x * frequency, y * frequency) * amplitude;
        max_amplitude += amplitude;
        
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }
    
    return result / max_amplitude;
}

CombinedNoise* combined_noise_create(uint32_t seed1, uint32_t seed2, int octaves) {
    CombinedNoise* noise = (CombinedNoise*)malloc(sizeof(CombinedNoise));
    noise->noise1 = octave_noise_create(seed1, octaves);
    noise->noise2 = octave_noise_create(seed2, octaves);
    return noise;
}

void combined_noise_free(CombinedNoise* noise) {
    octave_noise_free(noise->noise1);
    octave_noise_free(noise->noise2);
    free(noise);
}

float combined_noise_compute(CombinedNoise* noise, float x, float y) {
    float offset = octave_noise_compute(noise->noise2, x, y);
    return octave_noise_compute(noise->noise1, x + offset, y);
}
