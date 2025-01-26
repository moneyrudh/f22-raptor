#include "missile.h"
#include "renderer.h"
#include <math.h>

// Define the missile shape - it's a sleek arrowhead
static const SDL_Point MISSILE_SHAPE[] = {
    {8, 0},    // nose
    {-8, -4},  // left wing
    {-4, 0},   // body indent
    {-8, 4},   // right wing
    {8, 0}     // back to nose
};

static const int MISSILE_SHAPE_POINTS = 5;

MissileSystem missile_system_init(void) {
    MissileSystem system = {0};
    system.available_missiles = MAX_MISSILES;
    system.regen_timer = 0.0f;

    // Initialize missiles
    for (int i = 0; i < MAX_MISSILES; i++) {
        system.missiles[i].active = false;
        
        // Copy base shape to each missile
        for (int j = 0; j < MISSILE_SHAPE_POINTS; j++) {
            system.missiles[i].points[j] = MISSILE_SHAPE[j];
        }
        system.missiles[i].num_points = MISSILE_SHAPE_POINTS;
    }

    // Initialize UI
    const int CELL_WIDTH = 50;
    const int CELL_HEIGHT = 30;
    const int SPACING = 10;
    const int START_X = 20;
    const int START_Y = 20;

    for (int i = 0; i < MAX_MISSILES; i++) {
        SDL_Rect cell = {
            .x = START_X + i * (CELL_WIDTH + SPACING),
            .y = START_Y,
            .w = CELL_WIDTH,
            .h = CELL_HEIGHT
        };
        system.battery_cells[i] = cell;
    }

    return system;
}

void missile_system_fire(MissileSystem* system, const Player* player) {
    if (system->available_missiles <= 0) {
        printf("No missiles available to fire!\n");
        return;
    }

    // Find inactive missile slot
    Missile* missile = NULL;
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!system->missiles[i].active) {
            missile = &system->missiles[i];
            break;
        }
    }
    if (!missile) return;

    // Convert player angle to radians (player's 0° faces up)
    float angle = player->rotation * M_PI / 180.0f;
    float nose_distance = 35.0f;  // Distance from center to nose of plane
    
    // When plane points up (0°), offset should be (0, -nose_distance)
    // When plane tilts right (positive angle), we need trig to get components
    float offset_x = nose_distance * cosf(angle);     // sin gives x component
    float offset_y = -nose_distance * sinf(angle);    // -cos gives y component (negative because up is negative in SDL)
    
    // Position missile at plane's nose
    missile->x = f22_add(player->position.x, f22_from_float(offset_x));
    missile->y = f22_add(player->position.y, f22_from_float(offset_y));
    missile->rotation = player->rotation + 180;  // Add 90° to match visual direction
    missile->active = true;
    missile->trail_alpha = 1.0f;  // Start with full opacity trail

    // Calculate velocity (same as your working version)
    missile->velocity_x = MISSILE_SPEED * sinf(angle + M_PI_2);
    missile->velocity_y = -MISSILE_SPEED * cosf(angle + M_PI_2);

    system->available_missiles--;
    printf("Missile fired! Available: %d\n", system->available_missiles);
}

void missile_system_update(MissileSystem* system, const Player* player, AsteroidSystem* asteroids, float delta_time) {
    // Regeneration logic
    if (system->available_missiles < MAX_MISSILES) {
        system->regen_timer += delta_time;
        if (system->regen_timer >= MISSILE_REGEN_TIME) {
            system->available_missiles++;
            system->regen_timer = 0.0f;
        }
    }

    // Update active missiles
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!system->missiles[i].active) continue;

        // Update position
        system->missiles[i].x = f22_add(
            system->missiles[i].x,
            f22_from_float(system->missiles[i].velocity_x)
        );
        system->missiles[i].y = f22_add(
            system->missiles[i].y,
            f22_from_float(system->missiles[i].velocity_y)
        );

        // Fade trail over time
        system->missiles[i].trail_alpha = fmaxf(0.0f, system->missiles[i].trail_alpha - 0.05f);

        // Check asteroid collisions
        for (int j = 0; j < MAX_ASTEROIDS; j++) {
            if (!asteroids->asteroids[j].active) continue;

            float dx = f22_to_float(system->missiles[i].x) - f22_to_float(asteroids->asteroids[j].x);
            float dy = f22_to_float(system->missiles[i].y) - f22_to_float(asteroids->asteroids[j].y);
            float distance = sqrtf(dx * dx + dy * dy);

            float collision_radius = MISSILE_SIZE + (ASTEROID_BASE_SIZE * asteroids->asteroids[j].scale * 0.5f);
            if (distance < collision_radius) {
                system->missiles[i].active = false;
                asteroids->asteroids[j].active = false;
                break;
            }
        }

        // Check if off screen
        float x = f22_to_float(system->missiles[i].x);
        float y = f22_to_float(system->missiles[i].y);
        if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT) {
            system->missiles[i].active = false;
        }
    }
}

void missile_system_render(const MissileSystem* system, SDL_Renderer* renderer, F22 camera_y_offset) {
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!system->missiles[i].active) continue;

        ScreenPos pos = world_to_screen(system->missiles[i].x, system->missiles[i].y, camera_y_offset);
        float angle = system->missiles[i].rotation * M_PI / 180.0f;
        
        // Transform missile points
        SDL_Point transformed[MISSILE_SHAPE_POINTS];
        for (int j = 0; j < system->missiles[i].num_points; j++) {
            float px = system->missiles[i].points[j].x;
            float py = system->missiles[i].points[j].y;
            
            // Rotate point
            float rx = px * cosf(angle) - py * sinf(angle);
            float ry = px * sinf(angle) + py * cosf(angle);
            
            // Translate to position
            transformed[j].x = pos.x + (int)rx;
            transformed[j].y = pos.y + (int)ry;
        }

        // Draw missile trail
        SDL_SetRenderDrawColor(renderer, 255, 100, 50, (uint8_t)(255 * system->missiles[i].trail_alpha));
        
        // Calculate trail start point (back of missile)
        int trail_x = transformed[1].x + (transformed[2].x - transformed[1].x) / 2;
        int trail_y = transformed[1].y + (transformed[2].y - transformed[1].y) / 2;
        
        // Draw trail as a tapering line
        const int TRAIL_LENGTH = 30;
        for (int t = 0; t < TRAIL_LENGTH; t++) {
            float t_ratio = (float)t / TRAIL_LENGTH;
            float width = (1.0f - t_ratio) * 4.0f;  // trail gets thinner
            
            // Trail points extend opposite to velocity direction
            int x1 = trail_x - (int)(t * cosf(angle));
            int y1 = trail_y - (int)(t * sinf(angle));
            
            // Draw trail segment
            SDL_RenderDrawLine(
                renderer,
                x1 - (int)(width * sinf(angle)), y1 + (int)(width * cosf(angle)),
                x1 + (int)(width * sinf(angle)), y1 - (int)(width * cosf(angle))
            );
        }

        // Draw missile shape
        SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
        SDL_RenderDrawLines(renderer, transformed, system->missiles[i].num_points);
    }
}

void missile_system_render_ui(const MissileSystem* system, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    
    for (int i = 0; i < MAX_MISSILES; i++) {
        SDL_RenderDrawRect(renderer, &system->battery_cells[i]);
    }
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    for (int i = 0; i < system->available_missiles; i++) {
        SDL_Rect fillRect = {
            .x = system->battery_cells[i].x + 2,
            .y = system->battery_cells[i].y + 2,
            .w = system->battery_cells[i].w - 4,
            .h = system->battery_cells[i].h - 4
        };
        SDL_RenderFillRect(renderer, &fillRect);
    }
}