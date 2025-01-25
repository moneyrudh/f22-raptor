#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "f22.h"
#include <stdbool.h>
#include "wave.h"
#include "config.h"
#include "asteroid.h"
#include "player.h"
#include "explosion.h"
#include "smoke.h"

// Obstacle struct
typedef struct {
    F22 x;
    F22 gap_y;
    bool active;
} Obstacle;

// Game state struct
typedef struct {
    GameStateEnum state;
    F22 camera_y_offset;
    F22 target_y_offset;
    Player player;
    Obstacle obstacles[MAX_OBSTACLES];
    F22 last_obstacle_x;
    uint32_t score;
    WaveGenerator wave;
    AsteroidSystem asteroid_system;
    ExplosionSystem explosion;
    SmokeSystem smoke_system;
    ScoringSystem scoring;
} GameState;

// Player functions
Player player_init(void);
void player_update(Player* player, const GameState* state, bool thrust, float delta_time);
ScreenPos world_to_screen(F22 world_x, F22 world_y, F22 camera_y_offset);
ScreenPos player_get_screen_position(const Player* player, F22 camera_y_offset);

// Obstacle functions
Obstacle obstacle_init(void);
void obstacle_update(Obstacle* obstacle);
ScreenPos obstacle_get_screen_position(const Obstacle* obstacle);

// Game state functions
GameState game_state_init(void);
void game_state_start(GameState* state);
void game_state_handle_click(GameState* state, int x, int y);
void game_state_update(GameState* state, bool thrust_active, float delta_time);
bool game_state_check_collisions(GameState* state);

#endif // GAME_STATE_H
