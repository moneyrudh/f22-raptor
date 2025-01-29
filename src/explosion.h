#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <SDL.h>
#include "f22.h"
#include "player.h"
#include <stdbool.h>
#include <math.h>
#include "player.h"
#include "config.h"

#define MAX_DEBRIS 48
#define MAX_SPARKS 64
#define EXPLOSION_DURATION 2.0f  // seconds

typedef struct {
    SDL_Point points[8];  // shape points for this debris piece
    int num_points;
    float x, y;          // position
    float vx, vy;        // velocity
    float rotation;      // current rotation
    float rot_speed;     // rotation speed
    float scale;         // size scaling
    bool active;
    float lifetime;      // how long this piece has existed
    uint8_t r, g, b;     // color values
} Debris;

typedef struct {
    float x, y;
    float vx, vy;
    float lifetime;
    bool active;
    uint8_t r, g, b, a;  // color with alpha
} Spark;

typedef struct {
    Debris debris[MAX_DEBRIS];
    Spark sparks[MAX_SPARKS];
    bool active;
    float time;          // explosion timer
    F22 origin_x;        // where explosion started
    F22 origin_y;
} ExplosionSystem;

ExplosionSystem explosion_init(void);
void create_debris_piece(Debris* debris, const SDL_Point* shape, int num_points, float x, float y, float base_vx, float spread);
void create_spark(Spark* spark, float x, float y, float base_vx);
void explosion_start(ExplosionSystem* system, const Player* player);
void explosion_update(ExplosionSystem* system, float delta_time);
void explosion_render(const ExplosionSystem* system, SDL_Renderer* renderer, F22 camera_y_offset);
void explosion_reset(ExplosionSystem* system);

#endif