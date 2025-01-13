#include "game_state.h"
#include "config.h"
#include <math.h>
#include <stdio.h>

Player player_init(void) {
    Player player = {
        .position = {
            .x = f22_from_float(400),  // 20% from left
            .y = f22_from_float(300)  // middle of screen
        },
        .velocity = {
            .x = f22_from_float(0.0f),
            .y = f22_from_float(0.0f)
        },
        .rotation = 0.0f
    };
    return player;
}

void player_update(Player* player, bool thrust) {
    // Apply gravity
    player->velocity.y = f22_add(player->velocity.y, GRAVITY);

    // Apply thrust if active
    if (thrust) {
        player->velocity.y = f22_sub(player->velocity.y, THRUST);
    }

    // Update position
    player->position.y = f22_add(player->position.y, player->velocity.y);

    // Calculate rotation based on velocity
    float vel_y = f22_to_float(player->velocity.y);
    // Map velocity to rotation angle (-30 to 30 degrees)
    float target_rotation = -vel_y * 50.0f;
    // Smooth interpolation
    player->rotation = (player->rotation - target_rotation) * 0.1f;
    // Clamp rotation
    player->rotation = fmaxf(-30.0f, fminf(30.0f, player->rotation));

    // Clamp position
    // float pos_y = f22_to_float(player->position.y);
    // if (pos_y < f22_to_float(MIN_ALTITUDE)) {
    //     player->position.y = MIN_ALTITUDE;
    //     player->velocity.y = f22_from_float(0.0f);
    // } else if (pos_y > f22_to_float(MAX_ALTITUDE)) {
    //     player->position.y = MAX_ALTITUDE;
    //     player->velocity.y = f22_from_float(0.0f);
    // }
}

ScreenPos player_get_screen_position(const Player* player) {
    ScreenPos pos = {
        .x = (int)(f22_to_float(player->position.x) * WORLD_TO_SCREEN_SCALE),
        .y = (int)(f22_to_float(player->position.y) * WORLD_TO_SCREEN_SCALE)
    };
    return pos;
}

Obstacle obstacle_init(void) {
    Obstacle obstacle = {
        .x = f22_from_float(WINDOW_WIDTH),
        .gap_y = f22_from_float(50.0f),
        .active = false
    };
    return obstacle;
}

void obstacle_update(Obstacle* obstacle) {
    if (!obstacle->active) return;

    obstacle->x = f22_sub(obstacle->x, SCROLL_SPEED);

    // Deactivate if off screen
    if (f22_to_float(obstacle->x) < -OBSTACLE_WIDTH) {
        obstacle->active = false;
    }
}

ScreenPos obstacle_get_screen_position(const Obstacle* obstacle) {
    ScreenPos pos = {
        .x = (int)f22_to_float(obstacle->x),
        .y = (int)((1.0f - f22_to_float(obstacle->gap_y) / 100.0f) * WINDOW_HEIGHT)
    };
    return pos;
}

GameState game_state_init(void) {
    GameState state = {
        .player = player_init(),
        .last_obstacle_x = f22_from_float(WINDOW_WIDTH),
        .score = 0
    };

    // Initialize obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        state.obstacles[i] = obstacle_init();
    }

    return state;
}

void game_state_update(GameState* state, bool thrust_active) {
    // Update wave first
    wave_update(&state->wave);

    // Update player
    player_update(&state->player, thrust_active);

    // Note: Obstacle spawning commented out like in original
    // Update active obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (state->obstacles[i].active) {
            obstacle_update(&state->obstacles[i]);
        }
    }
}

bool game_state_check_collisions(GameState* state) {
    ScreenPos player_pos = player_get_screen_position(&state->player);
    const int player_radius = 15;  // Simplified collision circle

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!state->obstacles[i].active) continue;

        ScreenPos obs_pos = obstacle_get_screen_position(&state->obstacles[i]);

        // Check collision with vertical barriers
        if (player_pos.x + player_radius > obs_pos.x &&
            player_pos.x - player_radius < obs_pos.x + OBSTACLE_WIDTH) {

            // Check if outside gap
            if (player_pos.y - player_radius < obs_pos.y - 100 ||
                player_pos.y + player_radius > obs_pos.y + 100) {
                return true;
            }
        }
    }
    return false;
}
