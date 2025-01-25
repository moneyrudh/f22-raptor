#include "game_state.h"
#include "asteroid.h"
#include "player.h"
#include "config.h"
#include <math.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

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

    const float EDGE_BUFFER = 150.0f;
    const float RETURN_BUFFER = 200.0f;  // larger than EDGE_BUFFER

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

void player_update(Player* player, const GameState* state, bool thrust, float delta_time) {
    // Apply gravity
    // player->velocity.y = f22_add(player->velocity.y, 
    //     f22_mul(GRAVITY, f22_from_float(delta_time)));

    // if (thrust) {
    //     player->velocity.y = f22_sub(player->velocity.y, 
    //         f22_mul(THRUST, f22_from_float(delta_time)));
    // }

    player->velocity.y = f22_add(player->velocity.y, GRAVITY);
    if (thrust) {
        player->velocity.y = f22_sub(player->velocity.y, THRUST);
    }

    // Clamp vertical velocity
    float current_vel = f22_to_float(player->velocity.y);
    if (current_vel > f22_to_float(MAX_VELOCITY)) {
        player->velocity.y = MAX_VELOCITY;
    } else if (current_vel < f22_to_float(MIN_VELOCITY)) {
        player->velocity.y = MIN_VELOCITY;
    }

    // Update vertical position
    player->position.y = f22_add(player->position.y, player->velocity.y);


    // Calculate distance from ghost's y position
    int _x = (int) f22_to_float(state->player.position.x);
    float ghost_y = f22_to_float(state->wave.points[_x].y);
    float player_y = f22_to_float(player->position.y);
    float y_distance = fabsf(ghost_y - player_y);

    // Normalize the distance (0 to 1 scale)
    float normalized_distance = y_distance / WINDOW_HEIGHT;
    printf("NORMALIZED DISTANCE AND Y_DISTANCE %f, %f: ", normalized_distance, y_distance);

    float screen_mid = WINDOW_WIDTH / 2.0f;
    float player_x = f22_to_float(player->position.x);
    float exp_distance = normalized_distance * normalized_distance;  // x^2

    if (player_x < screen_mid) {
        // When left of mid-screen:
        // Close to ghost (small normalized_distance) = move right
        // Far from ghost (large normalized_distance) = move left
        float move_amount = (1.0f - normalized_distance) * 1.0f - normalized_distance * 6.0f;
        // float move_amount = 0.0f;
        // if (normalized_distance > 0.2f) {
        //     move_amount = 1.5f;
        // } else {
        //     move_amount = -2.0f;
        // }
        player->position.x = f22_add(player->position.x, f22_from_float(move_amount));
    } else {
        if (normalized_distance > 0.2f) {
            // Outside safe zone, move left fast
            player->position.x = f22_sub(player->position.x, f22_from_float(5.0f));
        } else {
            // Inside safe zone (within 20% of ghost), move left slowly
            player->position.x = f22_sub(player->position.x, f22_from_float(0.0f));
        }
    }

    // Calculate rotation based on velocity
    float vel_y = f22_to_float(player->velocity.y);
    // Map velocity to rotation angle (-30 to 30 degrees)
    float target_rotation = -vel_y * 50.0f;
    // Smooth interpolation
    player->rotation = (player->rotation - target_rotation) * 0.1f;
    // Clamp rotation
    player->rotation = fmaxf(-55.0f + SCROLL_SPEED, fminf(55.0f - SCROLL_SPEED, player->rotation));
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
        .state = GAME_STATE_WAITING,
        .player = player_init(),
        .last_obstacle_x = f22_from_float(WINDOW_WIDTH),
        .score = 0,
        .camera_y_offset = f22_from_float(0.0f),
        .target_y_offset = f22_from_float(0.0f),
        .wave = wave_init(),
        .asteroid_system = asteroid_system_init(),
        .explosion = explosion_init(),
        .smoke_system = smoke_system_init(),
        // .missile_system = missile_system_init(),
        .scoring = (ScoringSystem){
            .score = 0,
            .current_precision = 0,
            .score_rate = 0
        }
    };

    // Initialize obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        state.obstacles[i] = obstacle_init();
    }

    return state;
}

void game_state_start(GameState* state) {
    state->state = GAME_STATE_PLAYING;
    state->score = 0;
    
    printf("GAME HAS BEGUN LMFAO");
    // Reset player position to middle
    // state->player.position.x = f22_from_float(WINDOW_WIDTH / 2);
    // state->player.position.y = f22_from_float(WINDOW_HEIGHT / 2);
    // state->player.velocity.x = f22_from_float(0);
    // state->player.velocity.y = f22_from_float(0);
    
    // // Reset wave
    // state->wave = wave_init();
}

void game_state_handle_click(GameState* state, int x, int y) {
    if (state->state == GAME_STATE_WAITING) {
        game_state_start(state);
    }
}

void update_scoring(GameState* state) {
    // Calculate precision based on distance from wave
    float player_y = f22_to_float(state->player.position.y);
    int player_x = (int)f22_to_float(state->player.position.x);
    float wave_y = f22_to_float(state->wave.points[player_x].y);
    float distance = fabsf(wave_y - player_y);
    
    // Normalize to -1 to 1 where:
    // 1.0 = perfect alignment
    // 0.0 = moderate distance
    // -1.0 = too far
    float max_distance = WINDOW_HEIGHT * 0.3f; // adjust this threshold
    state->scoring.current_precision = 1.0f - (distance / max_distance);
    state->scoring.current_precision = fmaxf(-1.0f, fminf(1.0f, state->scoring.current_precision));

    // Calculate score rate based on precision
    if (state->scoring.current_precision > 0.8f) {
        state->scoring.score_rate = 1.25f;  // max rate
    } else if (state->scoring.current_precision > 0.3f) {
        state->scoring.score_rate = 0.5f;  // medium rate
    } else if (state->scoring.current_precision > -0.3f) {
        state->scoring.score_rate = -0.5f;  // base rate
    } else {
        state->scoring.score_rate = -2.0f; // penalty
    }

    // Update score (60fps assumed)
    state->scoring.score += state->scoring.score_rate / 60.0f;
    if (state->scoring.score < 0) state->scoring.score = 0;
}

void game_state_update(GameState* state, bool thrust_active, float delta_time) {
    if (state->state == GAME_STATE_WAITING) {
        state->player.position.x = f22_from_float(WINDOW_WIDTH / 2);
        state->player.position.y = f22_from_float(WINDOW_HEIGHT / 2);

        wave_update(&state->wave, WINDOW_HEIGHT / 2, state->state, delta_time);
        asteroid_system_update(&state->asteroid_system, &state->wave);
        return;
    }
    // Update wave first
    ScreenPos player_pos = player_get_screen_position(&state->player, state->camera_y_offset);
    wave_update(&state->wave, player_pos.y, state->state, delta_time);
    explosion_update(&state->explosion, delta_time);
    if (state->state == GAME_STATE_OVER) return;
    asteroid_system_update(&state->asteroid_system, &state->wave);

    // // Update missile system
    // missile_system_update(&state->missile_system, &state->player, &state->asteroid_system, delta_time);

    // Update player
    player_update(&state->player, state, thrust_active, delta_time);
    
    update_camera(state);
    update_scoring(state);

    #ifdef __EMSCRIPTEN__
        EM_ASM({
            Module.updateScore($0);
        }, (int)state->scoring.score);
    #endif

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
    if (state->state == GAME_STATE_OVER) return true;

    // Check if player hit left side
    if (f22_to_float(state->player.position.x) < GAME_OVER_X) {
        state->state = GAME_STATE_OVER;
        explosion_start(&state->explosion, &state->player);
        smoke_system_start(&state->smoke_system, &state->player);
        return true;
    }

    if (asteroid_system_check_collision(&state->asteroid_system, &state->player)) {
        state->state = GAME_STATE_OVER;
        explosion_start(&state->explosion, &state->player);
        smoke_system_start(&state->smoke_system, &state->player);
        return true;
    }

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
