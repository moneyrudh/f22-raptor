#include "renderer.h"
#include "asteroid.h"
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

    renderer->last_particle_spawn = SDL_GetTicks();

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer->renderer, &info);
    printf("RENDERER INFO: name=%s, flags=%d\n", info.name, info.flags);
    renderer_init_shapes(renderer);

    // if (TTF_Init() == -1) {
    //     printf("SDL_ttf could not initialize! Error: %s\n", TTF_GetError());
    //     return -1;
    // }

    // renderer_init_font(renderer, "/assets/vt323.ttf", 24);

    return 0;
}

// int renderer_init_font(Renderer* renderer, const char* font_path, int font_size) {
//     renderer->font = TTF_OpenFont(font_path, font_size);
//     if (!renderer->font) {
//         printf("Failed to load font! Error: %s\n", TTF_GetError());
//         return -1;
//     }
//     return 0;
// }

// void renderer_draw_text(Renderer* renderer, const char* text, int x, int y, SDL_Color color) {
//     if (!renderer->font) return;

//     SDL_Surface* text_surface = TTF_RenderText_Blended(renderer->font, text, color);
//     if (!text_surface) {
//         printf("Failed to render text! Error: %s\n", TTF_GetError());
//         return;
//     }

//     SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer->renderer, text_surface);
//     SDL_FreeSurface(text_surface);

//     if (!text_texture) {
//         printf("Failed to create texture! Error: %s\n", SDL_GetError());
//         return;
//     }

//     int text_width, text_height;
//     SDL_QueryTexture(text_texture, NULL, NULL, &text_width, &text_height);
//     SDL_Rect dest_rect = {x, y, text_width, text_height};
    
//     SDL_RenderCopy(renderer->renderer, text_texture, NULL, &dest_rect);
//     SDL_DestroyTexture(text_texture);
// }

// void renderer_cleanup(Renderer* renderer) {
//     if (renderer->font) {
//         TTF_CloseFont(renderer->font);
//     }
//     TTF_Quit();
//     SDL_DestroyRenderer(renderer->renderer);
//     SDL_DestroyWindow(renderer->window);
// }

void renderer_cleanup(Renderer* renderer) {
    SDL_DestroyRenderer(renderer->renderer);
    SDL_DestroyWindow(renderer->window);
}

void renderer_draw_score(Renderer* renderer, uint32_t score) {
    // char score_text[32];
    // snprintf(score_text, sizeof(score_text), "SCORE: %d", (int)scoring->score);
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

// void renderer_draw_wave(Renderer* renderer, const WaveGenerator* wave, F22 camera_y_offset) {
//     SDL_SetRenderDrawColor(renderer->renderer, 100, 200, 255, 255);
    
//     // Time-based animation
//     float time = SDL_GetTicks() / 1000.0f;
//     float wave_speed = 1.0f;  // controls how fast the wave moves
//     float amplitude = 45.0f;  // controls size of oscillation
//     float frequency = 0.05f;   // controls how tight the spiral is
    
//     renderer->num_wave_points = 0;
//     for (int i = 0; i < WINDOW_WIDTH; i++) {
//         if (wave->points[i].activated) {
//             ScreenPos base_pos = world_to_screen(wave->points[i].x, wave->points[i].y, camera_y_offset);
            
//             // Phase offset based on x position creates the moving spiral
//             float phase = i * frequency + time * wave_speed;
            
//             // Calculate offset for both x and y to create circular motion
//             // float x_offset = fabsf(amplitude * (float)sin(phase));
//             // float y_offset = fabsf(amplitude * (float)cos(phase));
//             float x_offset = (amplitude * (float)sin(phase));
//             float y_offset = (amplitude * (float)cos(phase));
//             // float x_offset = amplitude * sin(phase) + (amplitude/2) * sin(phase * 2);
//             // float y_offset = amplitude * cos(phase) + (amplitude/2) * cos(phase * 2);
            
//             renderer->wave_points[renderer->num_wave_points].x = base_pos.x + (int)x_offset;
//             renderer->wave_points[renderer->num_wave_points].y = base_pos.y + (int)y_offset;
//             renderer->num_wave_points++;
//         }
//     }

//     if (renderer->num_wave_points > 1) {
//         SDL_RenderDrawLines(renderer->renderer, renderer->wave_points, renderer->num_wave_points);
//     }
// }

static inline uint8_t lerp(uint8_t a, uint8_t b, float t) {
    return (uint8_t)(a + t * (b - a));
}

void renderer_draw_wave(Renderer* renderer, const WaveGenerator* wave, const Player* player, F22 camera_y_offset) {
    // SDL_SetRenderDrawColor(renderer->renderer, 255, 255, 255, 255);
    float time = SDL_GetTicks() / 1000.0f;
    float wave_speed = 2.0f;
    float max_amplitude = 25.0f;
    float frequency = 0.06f;
    
    // Get player position in screen coordinates for distance check
    ScreenPos player_pos = player_get_screen_position(player, camera_y_offset);
    const float EFFECT_RADIUS = 200.0f;  // how far the effect spreads from player
    const float TRANSITION_RADIUS = 100.0f; // smooth transition zone
    const float COLOR_RADIUS = 250.0f; 

    renderer->num_wave_points = 0;
    for (int i = 0; i < WINDOW_WIDTH; i++) {
        if (wave->points[i].activated) {
            ScreenPos base_pos = world_to_screen(wave->points[i].x, wave->points[i].y, camera_y_offset);
            
            // Calculate distance from this point to player
            float dx = base_pos.x - player_pos.x;
            float dy = base_pos.y - player_pos.y;
            float dist = sqrtf(dx * dx + dy * dy);
            
            // Smoothly fade the amplitude based on distance
            float amplitude_factor = 1.0f;
            if (dist > EFFECT_RADIUS - TRANSITION_RADIUS) {
                amplitude_factor = fmaxf(0.0f, 
                    1.0f - (dist - (EFFECT_RADIUS - TRANSITION_RADIUS)) / TRANSITION_RADIUS);
            }
            
            float phase = i * frequency + time * wave_speed;
            float x_offset = max_amplitude * amplitude_factor * sin(phase);
            float y_offset = max_amplitude * amplitude_factor * cos(phase);
            
            renderer->wave_points[renderer->num_wave_points].x = base_pos.x + (int)x_offset;
            renderer->wave_points[renderer->num_wave_points].y = base_pos.y + (int)y_offset;
            renderer->num_wave_points++;
        }
    }

    // Second pass: draw each segment with its own color
    for (int i = 0; i < renderer->num_wave_points - 1; i++) {
        // Calculate color for this segment (use midpoint between points)
        float mid_x = (renderer->wave_points[i].x + renderer->wave_points[i + 1].x) / 2.0f;
        float mid_y = (renderer->wave_points[i].y + renderer->wave_points[i + 1].y) / 2.0f;
        
        float dx = mid_x - player_pos.x;
        float dy = mid_y - player_pos.y;
        float dist = sqrtf(dx * dx + dy * dy);
        
        float amplitude_factor = 1.0f;
        if (dist > EFFECT_RADIUS - TRANSITION_RADIUS) {
            amplitude_factor = fmaxf(0.0f, 
                1.0f - (dist - (EFFECT_RADIUS - TRANSITION_RADIUS)) / TRANSITION_RADIUS);
        }
        
        // Calculate color based on x-distance
        float x_distance = fabsf(mid_x - player_pos.x);
        float color_factor = fmaxf(0.0f, 1.0f - x_distance / COLOR_RADIUS);
        
        // Combine phase and color factors
        float gradient_t = color_factor * amplitude_factor;
        
        // Cyberpunk color scheme (cyan to magenta)
        uint8_t r = amplitude_factor > 0.001f ? lerp(0, 255, amplitude_factor) : 10;    // cyan to magenta
        uint8_t g = amplitude_factor > 0.001f ? lerp(255, 0, amplitude_factor) : x_distance < COLOR_RADIUS * 2 ? 255 : 10;
        // uint8_t b = amplitude_factor > 0.001f ? lerp(255, 0, amplitude_factor) : x_distance < COLOR_RADIUS * 2 ? 255 : 0;
        uint8_t b = x_distance < COLOR_RADIUS * 2 ? 255 : 10;
        
        SDL_SetRenderDrawColor(renderer->renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer->renderer,
            renderer->wave_points[i].x, 
            renderer->wave_points[i].y,
            renderer->wave_points[i + 1].x, 
            renderer->wave_points[i + 1].y);
    }

    // if (renderer->num_wave_points > 1) {
    //     SDL_RenderDrawLines(renderer->renderer, 
    //         renderer->wave_points, 
    //         renderer->num_wave_points
    //     );
    // }
}

// void renderer_draw_wave(Renderer* renderer, const WaveGenerator* wave, F22 camera_y_offset) {
//     SDL_SetRenderDrawColor(renderer->renderer, 255, 255, 255, 255);

//     // Just copy the wave points directly to renderer points, shifting x coordinates left
//     renderer->num_wave_points = 0;
//     for (int i = 0; i < WINDOW_WIDTH; i++) {
//         if (wave->points[i].activated) {
//             ScreenPos pos = world_to_screen(wave->points[i].x, wave->points[i].y, camera_y_offset);
//             renderer->wave_points[renderer->num_wave_points].x = pos.x;
//             renderer->wave_points[renderer->num_wave_points].y = pos.y;
//             renderer->num_wave_points++;
//         }
//     }

//     if (renderer->num_wave_points > 1) {
//         SDL_RenderDrawLines(renderer->renderer, renderer->wave_points, renderer->num_wave_points);
//     }
// }

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

    if (state->state == GAME_STATE_WAITING) {
        // Draw simple waiting state
        // SDL_SetRenderDrawColor(renderer->renderer, 255, 255, 255, 255);
        // SDL_Rect prompt = {
        //     .x = (WINDOW_WIDTH / 2) - 100,
        //     .y = (WINDOW_HEIGHT / 2) - 50,
        //     .w = 200,
        //     .h = 40
        // };
        // SDL_RenderFillRect(renderer->renderer, &prompt);
        asteroid_system_render(&state->asteroid_system, renderer->renderer, state->camera_y_offset, &state->player);
    } else {
        // Normal game rendering
        // renderer_draw_obstacles(renderer, state->obstacles);
        renderer_draw_wave(renderer, &state->wave, &state->player, state->camera_y_offset);
        asteroid_system_render(&state->asteroid_system, renderer->renderer, state->camera_y_offset, &state->player);
        // missile_system_render(&state->missile_system, renderer->renderer, state->camera_y_offset);
    }

    // Always draw player and score
    missile_system_render_ui(&state->missile_system, renderer->renderer);
    renderer_draw_player(renderer, &state->player, state->camera_y_offset, state->state == GAME_STATE_PLAYING ? thrust_active : true);
    // renderer_draw_score(renderer, state->score);

    SDL_RenderPresent(renderer->renderer);

}
