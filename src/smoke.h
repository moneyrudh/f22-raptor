#ifndef SMOKE_H
#define SMOKE_H

#include "f22.h"
#include "player.h"
#include <stdbool.h>

#define MAX_PARTICLES 128

typedef struct {
    float x, y;
    float vx, vy;
    float size;
    float lifetime;
    float max_lifetime;
    uint8_t alpha;
    bool active;
} SmokeParticle;

typedef struct {
    SmokeParticle particles[MAX_PARTICLES];
    bool active;
    float time;
    F22 origin_x;
    F22 origin_y;
} SmokeSystem;

SmokeSystem smoke_system_init();
void create_particle(SmokeParticle* particle, float x, float y);
void smoke_system_start(SmokeSystem* system, const Player* player);
void smoke_system_update(SmokeSystem* system, float delta_time);
void smoke_system_reset(SmokeSystem* system);

#endif