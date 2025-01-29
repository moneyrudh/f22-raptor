#include "smoke.h"
#include "config.h"
#include <math.h>
#include <stdio.h>
#include <SDL.h>

SmokeSystem smoke_system_init(void) {
    SmokeSystem system;
    memset(&system, 0, sizeof(SmokeSystem));
    return system;
}

void smoke_system_reset(SmokeSystem* system) {
    // Reset main system state
    system->active = false;
    system->time = 0;
    system->origin_x = f22_from_float(0);
    system->origin_y = f22_from_float(0);
    
    // Deactivate all particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        system->particles[i].active = false;
    }
}

void create_particle(SmokeParticle* particle, float x, float y) {
    particle->active = true;
    particle->x = x;
    particle->y = y;
    
    // Random velocity in circle
    float angle = ((float)rand() / (float)RAND_MAX) * 2 * M_PI;
    float speed = 0.5f + ((float)rand() / (float)RAND_MAX) * 2.0f;
    particle->vx = cosf(angle) * speed;
    particle->vy = sinf(angle) * speed;
    
    // Random size and lifetime
    particle->size = 3.0f + ((float)rand() / (float)RAND_MAX) * 8.0f;
    particle->max_lifetime = 1.0f + ((float)rand() / (float)RAND_MAX) * 2.0f;
    particle->lifetime = 0;
    particle->alpha = 255;
}

void smoke_system_start(SmokeSystem* system, const Player* player) {
    if (system->active) return;
    
    system->active = true;
    system->time = 0;
    system->origin_x = player->position.x;
    system->origin_y = player->position.y;
    
    float x = f22_to_float(player->position.x);
    float y = f22_to_float(player->position.y);
    
    // Create initial burst of particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        create_particle(&system->particles[i], x, y);
    }
}

void smoke_system_update(SmokeSystem* system, float delta_time) {
    if (!system->active) return;
    
    system->time += delta_time;
    if (system->time >= 3.0f) {  // longer duration than explosion
        system->active = false;
        return;
    }
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        SmokeParticle* p = &system->particles[i];
        if (!p->active) continue;
        
        p->lifetime += delta_time;
        if (p->lifetime >= p->max_lifetime) {
            p->active = false;
            continue;
        }
        
        // Update position with slow down
        p->x += p->vx * (1.0f - p->lifetime / p->max_lifetime);
        p->y += p->vy * (1.0f - p->lifetime / p->max_lifetime);
        
        // Grow size over time
        p->size += delta_time * 5.0f;
        
        // Fade out
        float life_ratio = p->lifetime / p->max_lifetime;
        p->alpha = (uint8_t)(255.0f * (1.0f - life_ratio));
    }
}

ScreenPos __world_to_screen(F22 world_x, F22 world_y, F22 camera_y_offset) {
    ScreenPos pos = {
        .x = (int)(f22_to_float(world_x) * WORLD_TO_SCREEN_SCALE),
        .y = (int)((f22_to_float(world_y) - f22_to_float(camera_y_offset)) * WORLD_TO_SCREEN_SCALE)
    };
    return pos;
}

void smoke_system_render(const SmokeSystem* system, SDL_Renderer* renderer, F22 camera_y_offset) {
    if (!system->active) return;

    for (int i = 0; i < MAX_PARTICLES; i++) {
        const SmokeParticle* p = &system->particles[i];
        if (!p->active) continue;
        
        ScreenPos pos = __world_to_screen(
            f22_from_float(p->x),
            f22_from_float(p->y),
            camera_y_offset
        );
        
        // Draw smoke particle as a circle or filled rectangle
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, p->alpha);
        
        // Simple filled circle approximation
        int size = (int)p->size;
        for (int y = -size; y <= size; y++) {
            for (int x = -size; x <= size; x++) {
                if (x*x + y*y <= size*size) {
                    SDL_RenderDrawPoint(renderer, 
                        pos.x + x, 
                        pos.y + y
                    );
                }
            }
        }
    }
}