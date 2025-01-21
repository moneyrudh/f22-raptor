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

void update_camera(GameState* state) {
    float player_screen_y = f22_to_float(state->player.position.y) - f22_to_float(state->camera_y_offset);

    const float EDGE_BUFFER = 64.0f;
    const float RETURN_BUFFER = 128.0f;  // larger than EDGE_BUFFER

    // Start camera follow earlier with EDGE_BUFFER
    if (player_screen_y < EDGE_BUFFER) {
        state->target_y_offset = f22_sub(state->player.position.y, f22_from_float(EDGE_BUFFER));
    } else if (player_screen_y > WINDOW_HEIGHT - EDGE_BUFFER) {
        state->target_y_offset = f22_sub(state->player.position.y, f22_from_float(WINDOW_HEIGHT - EDGE_BUFFER));
    }
    // Only reset target if player is well within bounds
    else if (player_screen_y > RETURN_BUFFER && player_screen_y < WINDOW_HEIGHT - RETURN_BUFFER) {
        state->target_y_offset = state->camera_y_offset;  // maintain current offset
    }

    // Smooth camera movement using lerp
    F22 diff = f22_sub(state->target_y_offset, state->camera_y_offset);
    state->camera_y_offset = f22_add(state->camera_y_offset,
                                    f22_mul(diff, f22_from_float(0.5f))); // adjust 0.1 for smoother/faster
}

void player_update(Player* player, bool thrust) {
    // Apply gravity
    player->velocity.y = f22_add(player->velocity.y, GRAVITY);

    // Apply thrust if active
    if (thrust) {
        player->velocity.y = f22_sub(player->velocity.y, THRUST);
    }

    float current_vel = f22_to_float(player->velocity.y);
    if (current_vel > f22_to_float(MAX_VELOCITY)) {
        player->velocity.y = MAX_VELOCITY;
    } else if (current_vel < f22_to_float(MIN_VELOCITY)) {
        player->velocity.y = MIN_VELOCITY;
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

ScreenPos world_to_screen(F22 world_x, F22 world_y, F22 camera_y_offset) {
    ScreenPos pos = {
        .x = (int)(f22_to_float(world_x) * WORLD_TO_SCREEN_SCALE),
        .y = (int)((f22_to_float(world_y) - f22_to_float(camera_y_offset)) * WORLD_TO_SCREEN_SCALE)
    };
    return pos;
}

ScreenPos player_get_screen_position(const Player* player, F22 camera_y_offset) {
    // ScreenPos pos = {
    //     .x = (int)(f22_to_float(player->position.x) * WORLD_TO_SCREEN_SCALE),
    //     .y = (int)(f22_to_float(player->position.y) * WORLD_TO_SCREEN_SCALE)
    // };
    // return pos;
    return world_to_screen(player->position.x, player->position.y, camera_y_offset);
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

    obstacle->x = f22_sub(obstacle->x, f22_from_float(SCROLL_SPEED));

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
        .score = 0,
        .camera_y_offset = f22_from_float(0.0f),
        .target_y_offset = f22_from_float(0.0f),
        .wave = wave_init()
    };

    // Initialize obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        state.obstacles[i] = obstacle_init();
    }

    return state;
}

void game_state_update(GameState* state, bool thrust_active) {
    // Update wave first
    printf("Game state update called\n");
    ScreenPos player_pos = player_get_screen_position(&state->player, state->camera_y_offset);
    wave_update(&state->wave, player_pos.y);

    // Update player
    player_update(&state->player, thrust_active);
    update_camera(state);

    // Note: Obstacle spawning commented out like in original
    // Update active obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (state->obstacles[i].active) {
            obstacle_update(&state->obstacles[i]);
        }
    }
}

bool game_state_check_collisions(GameState* state) {
    ScreenPos player_pos = player_get_screen_position(&state->player, state->camera_y_offset);
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
