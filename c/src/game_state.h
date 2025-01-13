#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "f22.h"
#include <stdbool.h>
#include "wave.h"
#include "config.h"

// Game constants
#define OBSTACLE_WIDTH 50
#define OBSTACLE_GAP 200
#define MAX_OBSTACLES 5

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WORLD_TO_SCREEN_SCALE 1.0f

// Position struct to replace Zig's anonymous structs
typedef struct {
    F22 x;
    F22 y;
} Position;

// Player struct
typedef struct {
    Position position;
    Position velocity;
    float rotation;
} Player;

// Obstacle struct
typedef struct {
    F22 x;
    F22 gap_y;
    bool active;
} Obstacle;

// Screen position helpers
typedef struct {
    int x;
    int y;
} ScreenPos;

// Game state struct
typedef struct {
    Player player;
    Obstacle obstacles[MAX_OBSTACLES];
    F22 last_obstacle_x;
    uint32_t score;
    WaveGenerator wave;
} GameState;

// Player functions
Player player_init(void);
void player_update(Player* player, bool thrust);
ScreenPos player_get_screen_position(const Player* player);

// Obstacle functions
Obstacle obstacle_init(void);
void obstacle_update(Obstacle* obstacle);
ScreenPos obstacle_get_screen_position(const Obstacle* obstacle);

// Game state functions
GameState game_state_init(void);
void game_state_update(GameState* state, bool thrust_active);
bool game_state_check_collisions(GameState* state);

#endif // GAME_STATE_H
