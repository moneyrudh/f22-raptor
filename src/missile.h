#ifndef MISSILE_H
#define MISSILE_H

#include <SDL.h>
#include "f22.h"
#include "config.h"
#include "asteroid.h"
#include "player.h"

#define MAX_MISSILES 3
#define MISSILE_SPEED 15.0f
#define MISSILE_REGEN_TIME 5.0f
#define MISSILE_SIZE 8
#define MAX_MISSILE_POINTS 5  // matches our shape array size

typedef struct {
    F22 x;
    F22 y;
    float rotation;
    bool active;
    float velocity_x;
    float velocity_y;
    SDL_Point points[MAX_MISSILE_POINTS];  // points for missile shape
    int num_points;
    float trail_alpha;  // for trail fade effect
} Missile;

typedef struct {
    Missile missiles[MAX_MISSILES];
    int available_missiles;
    float regen_timer;
    SDL_Rect battery_cells[MAX_MISSILES];
} MissileSystem;

MissileSystem missile_system_init(void);
void missile_system_update(MissileSystem* system, const Player* player, AsteroidSystem* asteroids, float delta_time);
void missile_system_render(const MissileSystem* system, SDL_Renderer* renderer, F22 camera_y_offset);
void missile_system_fire(MissileSystem* system, const Player* player);
void missile_system_render_ui(const MissileSystem* system, SDL_Renderer* renderer);

#endif