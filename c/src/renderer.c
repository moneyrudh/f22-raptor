#include "renderer.h"
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int renderer_init(Renderer* renderer) {
    #ifdef __EMSCRIPTEN__
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN,
                              &renderer->window, &renderer->renderer);
    #else
    renderer->window = SDL_CreateWindow("F-22 Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!renderer->window) return -1;

    renderer->renderer = SDL_CreateRenderer(renderer->window, -1,
                                          SDL_RENDERER_ACCELERATED);
    #endif

    if (!renderer->renderer) return -1;

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer->renderer, &info);
    printf("RENDERER INFO: name=%s, flags=%d\n", info.name, info.flags);
    renderer_init_shapes(renderer);
    return 0;
}

void renderer_cleanup(Renderer* renderer) {
    SDL_DestroyRenderer(renderer->renderer);
    SDL_DestroyWindow(renderer->window);
}

void renderer_init_shapes(Renderer* renderer) {
    // Initialize F-22 shape points
    SDL_Point f22_base[] = {
        {50, 0},   // nose tip
        {45, -3},  // nose tip
        {40, -4},  // upper nose
        {32, -5},  // canopy start
        {30, -6},
        {26, -10}, // canopy peak
        {23, -11},
        {20, -12},
        {16, -12}, // canopy rear
        {15, -11},
        {14, -11},
        {-5, -6},  // canopy rear
        {-38, -4}, // main body
        {-45, -16}, // left tail top
        {-53, -16},
        {-54, 0},  // tail back
        {22, 4},   // wing root
        {18, 8},
        {0, 8},
        {-38, 16}, // wing tip
        {-48, 16}, // trailing edge
        {-48, 8},
        {-40, 2},  // rear fuselage
        {-60, 6},  // tail end
        {-66, 4},
        {-54, 0},
        {-10, 2},
        {22, 4},
        {27, 4},
        {40, 3},
        {48, 1},
        {50, 0}    // back to nose
    };

    // Copy points to renderer
    memcpy(renderer->f22_shape, f22_base, sizeof(f22_base));

    // Initialize thrust effect points (similar structure to original)
    SDL_Point thrust_base[] = {
        // Inner flame core
        {-60, 2}, {-64, 4}, {-85, 2},
        {-64, 4}, {-85, 6}, {-64, 4},
        {-60, 6},
        // Middle flame
        {-60, 0}, {-95, 4}, {-100, 4},
        {-95, 4}, {-60, 8},
        // Outer flame
        {-65, 4}, {-60, -2}, {-66, -3},
        {-70, -3}, {-80, -2}, {-95, 2},
        {-100, 4}, {-125, 4}, {-100, 4},
        {-95, 6}, {-80, 10}, {-70, 11},
        {-66, 11}, {-60, 10}, {-65, 4}
    };

    memcpy(renderer->thrust_shape, thrust_base, sizeof(thrust_base));

    SDL_Point left_wing[] = {
        {-5, -6},   // wing root
        {-12, -12}, // wing mid
        {-20, -12}, // wing tip
        {-32, -4}   // wing tip
    };

    memcpy(renderer->left_wing, left_wing, sizeof(left_wing));

    SDL_Point left_tail[] = {
        {-32, -4},  // wing root
        {-35, -20}, // wing mid
        {-41, -20}, // wing tip
        {-43, -11}, // wing tip
        {-38, -4},  // wing tip
        {-32, -4}   // wing tip
    };

    memcpy(renderer->left_tail, left_tail, sizeof(left_tail));

    SDL_Point cock_pit[] = {
        {28, -8},
        {24, -6},
        {20, -6},
        {12, -7},
        {8, -9}
    };

    memcpy(renderer->cock_pit, cock_pit, sizeof(cock_pit));
}

void renderer_draw_wave(Renderer* renderer, const WaveGenerator* wave, F22 camera_y_offset) {
    SDL_SetRenderDrawColor(renderer->renderer, 100, 200, 255, 255);

    // Just copy the wave points directly to renderer points, shifting x coordinates left
    renderer->num_wave_points = WINDOW_WIDTH;
    for (int i = 0; i < WINDOW_WIDTH; i++) {
        ScreenPos pos = world_to_screen(wave->points[i].x, wave->points[i].y, camera_y_offset);
        renderer->wave_points[i].x = pos.x;
        renderer->wave_points[i].y = pos.y;
        // renderer->wave_points[i].x = (int)(f22_to_float(wave->points[i].x));
        // renderer->wave_points[i].y = (int)f22_to_float(wave->points[i].y);
    }

    if (renderer->num_wave_points > 1) {
        SDL_RenderDrawLines(renderer->renderer, renderer->wave_points, renderer->num_wave_points);
    }
}

void renderer_rotate_points(SDL_Point* points, int num_points, SDL_Point center, float angle) {
    float angle_rad = angle * M_PI / 180.0f;
    float cos_angle = cosf(angle_rad);
    float sin_angle = sinf(angle_rad);

    for (int i = 0; i < num_points; i++) {
        float dx = (float)points[i].x;
        float dy = (float)points[i].y;

        // Rotate around origin
        float rotated_x = dx * cos_angle - dy * sin_angle;
        float rotated_y = dx * sin_angle + dy * cos_angle;

        // Translate back to screen position
        points[i].x = center.x + (int)rotated_x;
        points[i].y = center.y + (int)rotated_y;
    }
}

void renderer_draw_player(Renderer* renderer, const Player* player, F22 camera_y_offset, bool thrust_active) {
    ScreenPos pos = player_get_screen_position(player, camera_y_offset);
    SDL_Point center = {pos.x, pos.y};
    // Create temporary arrays for rotated points
    SDL_Point rotated_f22[32];
    memcpy(rotated_f22, renderer->f22_shape, sizeof(renderer->f22_shape));
    renderer_rotate_points(rotated_f22, 32, center, player->rotation);

    // Draw F-22 shape
    SDL_SetRenderDrawColor(renderer->renderer, 230, 230, 230, 255);
    SDL_RenderDrawLines(renderer->renderer, rotated_f22, 32);

    SDL_Point rotated_left_wing[4];
    memcpy(rotated_left_wing, renderer->left_wing, sizeof(renderer->left_wing));
    renderer_rotate_points(rotated_left_wing, 4, center, player->rotation);
    SDL_RenderDrawLines(renderer->renderer, rotated_left_wing, 4);

    SDL_Point rotated_left_tail[6];
    memcpy(rotated_left_tail, renderer->left_tail, sizeof(renderer->left_tail));
    renderer_rotate_points(rotated_left_tail, 6, center, player->rotation);
    SDL_RenderDrawLines(renderer->renderer, rotated_left_tail, 6);

    SDL_Point rotated_cock_pit[5];
    memcpy(rotated_cock_pit, renderer->cock_pit, sizeof(renderer->cock_pit));
    renderer_rotate_points(rotated_cock_pit, 5, center, player->rotation);
    SDL_RenderDrawLines(renderer->renderer, rotated_cock_pit, 5);

    if (thrust_active) {
        SDL_Point rotated_thrust[27];
        memcpy(rotated_thrust, renderer->thrust_shape, sizeof(renderer->thrust_shape));
        renderer_rotate_points(rotated_thrust, 27, center, player->rotation);

        // Draw thrust effects
        SDL_SetRenderDrawColor(renderer->renderer, 255, 100, 0, 255);
        SDL_RenderDrawLines(renderer->renderer, rotated_thrust, 7);

        SDL_SetRenderDrawColor(renderer->renderer, 255, 150, 50, 255);
        SDL_RenderDrawLines(renderer->renderer, rotated_thrust + 7, 5);

        SDL_SetRenderDrawColor(renderer->renderer, 255, 200, 0, 255);
        SDL_RenderDrawLines(renderer->renderer, rotated_thrust + 12, 15);
    }
}

void renderer_draw_obstacles(Renderer* renderer, const Obstacle* obstacles) {
    SDL_SetRenderDrawColor(renderer->renderer, 150, 150, 150, 255);

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;

        ScreenPos pos = obstacle_get_screen_position(&obstacles[i]);

        // Draw top obstacle
        SDL_Rect top_rect = {
            .x = pos.x,
            .y = 0,
            .w = OBSTACLE_WIDTH,
            .h = pos.y - 100
        };
        SDL_RenderFillRect(renderer->renderer, &top_rect);

        // Draw bottom obstacle
        SDL_Rect bottom_rect = {
            .x = pos.x,
            .y = pos.y + 100,
            .w = OBSTACLE_WIDTH,
            .h = WINDOW_HEIGHT - (pos.y + 100)
        };
        SDL_RenderFillRect(renderer->renderer, &bottom_rect);
    }
}

void renderer_draw_frame(Renderer* renderer, const GameState* state, bool thrust_active) {
    // Clear screen
    SDL_SetRenderDrawColor(renderer->renderer, 10, 10, 10, 255);
    SDL_RenderClear(renderer->renderer);

    renderer_draw_wave(renderer, &state->wave, state->camera_y_offset);

    // Draw game elements
    renderer_draw_obstacles(renderer, state->obstacles);
    renderer_draw_player(renderer, &state->player, state->camera_y_offset, thrust_active);

    SDL_RenderPresent(renderer->renderer);
}
