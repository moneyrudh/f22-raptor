#include "renderer.h"
#include "asteroid.h"
#include "player.h"
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
    renderer->background = background_init(renderer->renderer);

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer->renderer, &info);
    printf("RENDERER INFO: name=%s, flags=%d\n", info.name, info.flags);
    renderer_init_shapes(renderer);
    SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_BLEND);
    // if (TTF_Init() == -1) {
    //     printf("SDL_ttf could not initialize! Error: %s\n", TTF_GetError());
    //     return -1;
    // }

    // renderer_init_font(renderer, "/assets/vt323.ttf", 24);

    return 0;
}

Background* background_init(SDL_Renderer* renderer) {
    static Background bg;
    
    // Create a texture the size of our window
    bg.star_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Set initial star positions
    for(int i = 0; i < 50; i++) {
        bg.stars[i] = (Star){
            .x = rand() % WINDOW_WIDTH,
            .y = rand() % WINDOW_HEIGHT,
            .size = 1 + (rand() % 5),
            .speed = 1 + (rand() % 2),
            .brightness = 150 + (rand() % 100)
        };
    }
    
    // Draw initial star positions to texture
    update_star_texture(renderer, &bg);
    
    bg.gradient_opacity = 0;
    bg.gradient_direction = 1;
    bg.last_frame = SDL_GetTicks();
    return &bg;
}

void update_star_texture(SDL_Renderer* renderer, Background* bg) {
    // Switch render target to our texture
    SDL_SetRenderTarget(renderer, bg->star_texture);
    
    // Clear the texture
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    
    // Draw all stars to the texture
    for(int i = 0; i < 50; i++) {
        int _rand = rand() % 2;
        if (_rand == 0) {
            SDL_SetRenderDrawColor(renderer, 
                bg->stars[i].brightness,
                bg->stars[i].brightness,
                bg->stars[i].brightness,
                255);
        } else {
            SDL_SetRenderDrawColor(renderer, 
                bg->stars[i].brightness,
                bg->stars[i].brightness / 5,
                255,
                255);
        }
            
        if(bg->stars[i].size > 1) {
            SDL_Rect star = {
                (int)bg->stars[i].x,
                (int)bg->stars[i].y,
                2, 2
            };
            SDL_RenderFillRect(renderer, &star);
        } else {
            SDL_RenderDrawPoint(renderer,
                (int)bg->stars[i].x,
                (int)bg->stars[i].y);
        }
    }
    
    // Switch back to default render target
    SDL_SetRenderTarget(renderer, NULL);
}

void draw_background(SDL_Renderer* renderer, Background* bg, const GameState* state) {
    uint32_t current_time = SDL_GetTicks();
    float delta = (current_time - bg->last_frame) / 1000.0f;
    bg->last_frame = current_time;

    // Don't update stars if game is over
    if (state->state != GAME_STATE_OVER) {
        static int update_counter = 0;
        if(++update_counter >= 3) {
            // Calculate player's movement influence
            float player_x = f22_to_float(state->player.position.x);
            float player_movement = 0.0f;
            
            if (player_x < WINDOW_WIDTH / 2.0f) {
                // Calculate normalized distance from ghost path
                float ghost_y = f22_to_float(state->wave.points[(int)player_x].y);
                float player_y = f22_to_float(state->player.position.y);
                float y_distance = fabsf(ghost_y - player_y) / WINDOW_HEIGHT;
                
                // Player moves faster left when far from path, and right when close
                player_movement = (1.0f - y_distance) * 1.0f - y_distance * 6.0f;
            }
            
            // Update star positions with combined movement
            for(int i = 0; i < 50; i++) {
                // Base movement plus player influence
                float total_speed = fmaxf(-0.1f, bg->stars[i].speed + player_movement);
                bg->stars[i].x -= total_speed;
                
                if(bg->stars[i].x < 0) {
                    bg->stars[i].x = WINDOW_WIDTH;
                    bg->stars[i].y = rand() % WINDOW_HEIGHT;
                }
            }
            update_star_texture(renderer, bg);
            update_counter = 0;
        }
    }
    
    // Draw gradient background
    SDL_Rect bg_rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    uint8_t alpha = (uint8_t)(bg->gradient_opacity * 25);
    SDL_SetRenderDrawColor(renderer, 20, 15, 35, alpha);
    SDL_RenderFillRect(renderer, &bg_rect);
    
    // Draw star texture
    SDL_RenderCopy(renderer, bg->star_texture, NULL, NULL);
}

void renderer_cleanup(Renderer* renderer) {
    SDL_DestroyRenderer(renderer->renderer);
    SDL_DestroyWindow(renderer->window);
    if (renderer->background) {
        SDL_DestroyTexture(renderer->background->star_texture);
        free(renderer->background);
    }
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

    const int CIRCLE_X = 20;     // Position within cockpit area
    const int CIRCLE_Y = -8;     // Slightly below cockpit top
    const int RADIUS = 2;       // Small radius to fit cockpit
    const int NUM_SEGMENTS = 16; // Number of segments for circle approximation
    
    // Generate circle points using sine/cosine
    for(int i = 0; i < NUM_SEGMENTS; i++) {
        float angle = (float)i * 2.0f * M_PI / NUM_SEGMENTS;
        renderer->pilot_circle[i].x = CIRCLE_X + (int)(RADIUS * cosf(angle));
        renderer->pilot_circle[i].y = CIRCLE_Y + (int)(RADIUS * sinf(angle));
    }
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

// Simple 1D perlin-like noise for smooth random movement
float smooth_noise(float y, float time, float frequency, float camera_offset) {
    float scaled_offset = camera_offset * 0.55f;  // adjust this multiplier to taste
    
    // Add camera offset to both y and time for a flowing effect
    float y_offset = (y + scaled_offset) * frequency;
    float t_offset = time * 2.0f;
    
    return sinf(y_offset) * 0.3f + 
           sinf(y_offset * 1.7f + t_offset) * 0.2f +
           sinf(y_offset * 2.3f - t_offset * 0.8f) * 0.1f;
}

void renderer_draw_barrier(SDL_Renderer* renderer, float time, F22 camera_y_offset) {
    const int NUM_POINTS = 200;  // number of points per line
    const int BASE_X = GAME_OVER_X - 20;  // base x position for both lines
    const int LINE_SPACING = 25;  // space between the two lines
    SDL_Point line1[NUM_POINTS];
    SDL_Point line2[NUM_POINTS];
    
    // Generate points for both lines with smooth noise offsets
    for(int i = 0; i < NUM_POINTS; i++) {
        float y = (float)i * WINDOW_HEIGHT / (NUM_POINTS - 1);
        
        // Calculate noise offsets for each line
        float camera_y = f22_to_float(camera_y_offset);
        float noise1 = smooth_noise(y, time, 0.03f, camera_y) * 50.0f;
        float noise2 = smooth_noise(y, time + 100.0f, 0.03f, camera_y) * 50.0f;
        
        // Set points with noise offset
        line1[i].x = BASE_X + (int)noise1;
        line1[i].y = (int)y;
        
        line2[i].x = BASE_X + LINE_SPACING + (int)noise2;
        line2[i].y = (int)y;
    }
    
    // Draw the lines with the wave color scheme
    for(int i = 0; i < NUM_POINTS - 1; i++) {
        // Calculate color based on height (like the wave)
        float height_factor = (float)i / NUM_POINTS;
        uint8_t r = lerp(0, 255, height_factor);
        uint8_t g = lerp(255, 0, height_factor);
        uint8_t b = 255;
        
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        
        // Draw line segments
        SDL_RenderDrawLine(renderer, 
            line1[i].x, line1[i].y, 
            line1[i+1].x, line1[i+1].y);
        SDL_RenderDrawLine(renderer, 
            line2[i].x, line2[i].y, 
            line2[i+1].x, line2[i+1].y);
            
        // Draw connecting segments occasionally for "energy" effect
        // if(i % 20 == 0) {
        //     SDL_SetRenderDrawColor(renderer, r, g, b, 100);
        //     SDL_RenderDrawLine(renderer,
        //         line1[i].x, line1[i].y,
        //         line2[i].x, line2[i].y);
        // }
    }
}
void renderer_end_wave(Renderer* renderer, const WaveGenerator* wave, const Player* player, F22 camera_y_offset) {
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

    for (int i = 0; i < WINDOW_WIDTH - 1; i++) {
        if (wave->points[i].activated) {
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
    }
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
    for (int i = 0; i < renderer->num_wave_points - 2; i++) {
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

void renderer_draw_thrust(SDL_Renderer* sdl_renderer, const SDL_Point center, float rotation, float time, const SDL_Point* thrust_shape) {
    SDL_Point animated_thrust[27];  // same size as original thrust array
    
    // Copy the base thrust points
    memcpy(animated_thrust, thrust_shape, 27 * sizeof(SDL_Point));
    
    // Animate the points before rotation
    float wave_speed = 15.0f;  // controls how fast the flames "flicker"
    float wave_size = 3.0f;   // controls how much the flames move
    
    // Inner flame (points 0-6)
    for(int i = 0; i < 7; i++) {
        // Add some vertical waviness
        float phase = time * wave_speed + i * 0.5f;
        animated_thrust[i].y += (int)(wave_size * sinf(phase));
        
        // Randomly adjust length
        if(i % 2 == 1) {  // only adjust every other point for "stretchy" effect
            animated_thrust[i].x -= (rand() % 5) - 2;
        }
    }
    
    // Middle flame (points 7-11)
    for(int i = 7; i < 12; i++) {
        float phase = time * wave_speed * 1.2f + i * 0.3f;
        animated_thrust[i].y += (int)(wave_size * 1.5f * sinf(phase));
        if(i % 2 == 1) {
            animated_thrust[i].x -= (rand() % 8) - 3;
        }
    }
    
    // Outer flame (points 12-26)
    for(int i = 12; i < 27; i++) {
        float phase = time * wave_speed * 0.8f + i * 0.2f;
        animated_thrust[i].y += (int)(wave_size * 2.0f * sinf(phase));
        if(i % 2 == 1) {
            animated_thrust[i].x -= (rand() % 10) - 4;
        }
    }
    
    // Now rotate all points
    renderer_rotate_points(animated_thrust, 27, center, rotation);

    // Draw the animated flames with the original color scheme
    SDL_SetRenderDrawColor(sdl_renderer, 255, 100, 0, 255);
    SDL_RenderDrawLines(sdl_renderer, animated_thrust, 7);

    SDL_SetRenderDrawColor(sdl_renderer, 255, 150, 50, 255);
    SDL_RenderDrawLines(sdl_renderer, animated_thrust + 7, 5);

    SDL_SetRenderDrawColor(sdl_renderer, 255, 200, 0, 255);
    SDL_RenderDrawLines(sdl_renderer, animated_thrust + 12, 15);

    // Add some random spark particles
    SDL_SetRenderDrawColor(sdl_renderer, 255, 255, 200, 255);
    for(int i = 0; i < 5; i++) {
        // Generate spark position near the engine
        float spark_angle = ((float)rand() / (float)RAND_MAX) * M_PI - M_PI/2;  // -90 to 90 degrees
        float distance = 60 + (rand() % 40);  // 60-100 pixels from center
        
        float cos_rot = cosf((rotation + 180) * M_PI / 180.0f);  // +180 to point backwards
        float sin_rot = sinf((rotation + 180) * M_PI / 180.0f);
        
        int spark_x = center.x + (int)(distance * cos_rot);
        int spark_y = center.y + (int)(distance * sin_rot);
        
        // Draw a small cross for each spark
        SDL_RenderDrawLine(sdl_renderer, spark_x-1, spark_y-1, spark_x+1, spark_y+1);
        SDL_RenderDrawLine(sdl_renderer, spark_x-1, spark_y+1, spark_x+1, spark_y-1);
    }
}

void renderer_draw_player(Renderer* renderer, const Player* player, F22 camera_y_offset, bool thrust_active) {
    ScreenPos pos = player_get_screen_position(player, camera_y_offset);
    SDL_Point center = {pos.x, pos.y};

    // Create temporary arrays for rotated points
    SDL_Point rotated_f22[32];
    SDL_Point rotated_left_wing[4];
    SDL_Point rotated_left_tail[6];
    SDL_Point rotated_cock_pit[5];
    SDL_Point rotated_pilot[16];

    // First copy all the points
    memcpy(rotated_f22, renderer->f22_shape, sizeof(renderer->f22_shape));
    memcpy(rotated_left_wing, renderer->left_wing, sizeof(renderer->left_wing));
    memcpy(rotated_left_tail, renderer->left_tail, sizeof(renderer->left_tail));
    memcpy(rotated_cock_pit, renderer->cock_pit, sizeof(renderer->cock_pit));
    memcpy(rotated_pilot, renderer->pilot_circle, sizeof(renderer->pilot_circle));

    // Then rotate ALL points first
    renderer_rotate_points(rotated_f22, 32, center, player->rotation);
    renderer_rotate_points(rotated_left_wing, 4, center, player->rotation);
    renderer_rotate_points(rotated_left_tail, 6, center, player->rotation);
    renderer_rotate_points(rotated_cock_pit, 5, center, player->rotation);
    renderer_rotate_points(rotated_pilot, 16, center, player->rotation);

    // Now fill with the rotated points
    SDL_SetRenderDrawColor(renderer->renderer, 225, 225, 225, 255);
    fill_polygon(renderer->renderer, rotated_f22, 32);
    fill_polygon(renderer->renderer, rotated_left_wing, 4);
    fill_polygon(renderer->renderer, rotated_left_tail, 6);
    fill_polygon(renderer->renderer, rotated_cock_pit, 5);

    // Finally draw outlines
    SDL_SetRenderDrawColor(renderer->renderer, 50, 50, 50, 255);
    SDL_RenderDrawLines(renderer->renderer, rotated_f22, 32);
    SDL_RenderDrawLines(renderer->renderer, rotated_left_wing, 4);
    SDL_RenderDrawLines(renderer->renderer, rotated_left_tail, 6);
    SDL_RenderDrawLines(renderer->renderer, rotated_cock_pit, 5);




    
    // Draw pilot with a different color (maybe dark gray to show silhouette)
    SDL_SetRenderDrawColor(renderer->renderer, 50, 50, 50, 255);
    // SDL_SetRenderDrawColor(renderer->renderer, 250, 250, 250, 255);
    for(int i = 0; i < 15; i++) {
        SDL_RenderDrawLine(renderer->renderer, 
            rotated_pilot[i].x, rotated_pilot[i].y,
            rotated_pilot[i + 1].x, rotated_pilot[i + 1].y);
    }
    // Connect last point to first to close the circle
    SDL_RenderDrawLine(renderer->renderer,
        rotated_pilot[15].x, rotated_pilot[15].y,
        rotated_pilot[0].x, rotated_pilot[0].y);

    if (thrust_active) {
        float time = SDL_GetTicks() / 1000.0f;  // get current time for animation
        renderer_draw_thrust(renderer->renderer, center, player->rotation, time, renderer->thrust_shape);
        // SDL_Point rotated_thrust[27];
        // memcpy(rotated_thrust, renderer->thrust_shape, sizeof(renderer->thrust_shape));
        // renderer_rotate_points(rotated_thrust, 27, center, player->rotation);

        // // Draw thrust effects
        // SDL_SetRenderDrawColor(renderer->renderer, 255, 100, 0, 255);
        // SDL_RenderDrawLines(renderer->renderer, rotated_thrust, 7);

        // SDL_SetRenderDrawColor(renderer->renderer, 255, 150, 50, 255);
        // SDL_RenderDrawLines(renderer->renderer, rotated_thrust + 7, 5);

        // SDL_SetRenderDrawColor(renderer->renderer, 255, 200, 0, 255);
        // SDL_RenderDrawLines(renderer->renderer, rotated_thrust + 12, 15);
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
    draw_background(renderer->renderer, renderer->background, state);

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
        if (state->state == GAME_STATE_PLAYING) {
            renderer_draw_wave(renderer, &state->wave, &state->player, state->camera_y_offset);
        } else {
            renderer_end_wave(renderer, &state->wave, &state->player, state->camera_y_offset);
        }
        
        asteroid_system_render(&state->asteroid_system, renderer->renderer, state->camera_y_offset, &state->player);
        // missile_system_render(&state->missile_system, renderer->renderer, state->camera_y_offset);
    }

    // Always draw player and score
    // missile_system_render_ui(&state->missile_system, renderer->renderer);
    float time = SDL_GetTicks() / 1000.0f;
    renderer_draw_barrier(renderer->renderer, time, state->camera_y_offset);
    if (!state->explosion.active) {
        renderer_draw_player(renderer, &state->player, state->camera_y_offset, state->state == GAME_STATE_PLAYING ? thrust_active : true);
    }
    explosion_render(&state->explosion, renderer->renderer, state->camera_y_offset);
    // renderer_draw_score(renderer, state->score);

    SDL_RenderPresent(renderer->renderer);

}
