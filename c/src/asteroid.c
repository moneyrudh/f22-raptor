#include "asteroid.h"
#include "renderer.h"
#include <math.h>

#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)

static const SDL_Point ASTEROID_SHAPE[MAX_ASTEROID_POINTS] = {
    {20, 0},   {15, -15}, {0, -20},  {-15, -15},
    {-20, 0},  {-15, 10}, {-5, 20},  {5, 20},
    {15, 10},  {20, 0}    // last point connects back to first
};

static void generate_asteroid(Asteroid* asteroid, float scale, F22 x, F22 y) {
    asteroid->x = x;
    asteroid->y = y;
    asteroid->scale = scale;
    asteroid->rotation = ((float)rand() / (float)RAND_MAX) * 360.0f;
    asteroid->active = true;
    asteroid->num_points = 10;  // matches ASTEROID_SHAPE

    // Copy and scale base shape
    for (int i = 0; i < asteroid->num_points; i++) {
        asteroid->points[i].x = ASTEROID_SHAPE[i].x * scale;
        asteroid->points[i].y = ASTEROID_SHAPE[i].y * scale;
    }
}

AsteroidSystem asteroid_system_init(void) {
    AsteroidSystem system;
    memset(&system, 0, sizeof(AsteroidSystem));

    // Copy base shape
    memcpy(system.base_shape, ASTEROID_SHAPE, sizeof(ASTEROID_SHAPE));

    // Initialize all asteroids as inactive
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        system.asteroids[i].active = false;
    }

    return system;
}

static void spawn_asteroid_layer(AsteroidSystem* system, const WaveGenerator* wave, float layer_multiplier) {
    // Randomly decide spawn pattern (0 = above, 1 = below, 2 = both)
    int spawn_pattern = rand() % 3;

    if (spawn_pattern == 0 || spawn_pattern == 2) {
        spawn_asteroid(system, wave, true, layer_multiplier);  // spawn above
    }
    if (spawn_pattern == 1 || spawn_pattern == 2) {
        spawn_asteroid(system, wave, false, layer_multiplier); // spawn below
    }
}

static void spawn_asteroid(AsteroidSystem* system, const WaveGenerator* wave, bool spawn_above, float layer_multiplier) {
    // Find inactive asteroid slot
    Asteroid* asteroid = NULL;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!system->asteroids[i].active) {
            asteroid = &system->asteroids[i];
            break;
        }
    }
    if (!asteroid) return;  // no free slots

    // Check if enough space from last asteroid
    // for (int i = 0; i < MAX_ASTEROIDS; i++) {
    //     if (system->asteroids[i].active) {
    //         float dx = f22_to_float(system->asteroids[i].x) - WINDOW_WIDTH;
    //         if (fabsf(dx) < MIN_ASTEROID_SPACING) return;
    //     }
    // }

    float x_offset = 0.0f;// 25.0f + (25.0f * (float)rand() / (float)RAND_MAX);
    int spawn_x = WINDOW_WIDTH + ASTEROID_BASE_SIZE + (int)x_offset;

    // Get ghost Y at actual spawn point if it's within our simulated range
    float ghost_y;
    if (spawn_x < GHOST_WIDTH) {
        ghost_y = f22_to_float(wave->points[spawn_x].y);
    } else {
        spawn_x = GHOST_WIDTH - 1;
        // Fall back to last known position if beyond simulation
        ghost_y = f22_to_float(wave->points[spawn_x].y);
    }
    // Randomly choose above or below path
    float scale = MIN_ASTEROID_SCALE +
                 ((float)rand() / (float)RAND_MAX) * (MAX_ASTEROID_SCALE - MIN_ASTEROID_SCALE);

    // Calculate min and max offset distances
    float min_offset = (ASTEROID_SPAWN_BUFFER + ASTEROID_BASE_SIZE * scale) * layer_multiplier;
    float max_offset = (WINDOW_HEIGHT / 2) * layer_multiplier;
    float y_offset = min_offset + ((float)rand() / (float)RAND_MAX) * (max_offset - min_offset);
    int direction = spawn_above ? -1 : 1;
    y_offset *= direction;

    generate_asteroid(
        asteroid,
        scale,
        f22_from_float(spawn_x),
        f22_from_float(ghost_y + y_offset)
    );
}

void asteroid_system_update(AsteroidSystem* system, const WaveGenerator* wave) {
    // Update existing asteroids
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!system->asteroids[i].active) continue;

        // Move left at scroll speed
        system->asteroids[i].x = f22_sub(
            system->asteroids[i].x,
            f22_from_float(SCROLL_SPEED)
        );

        // Deactivate if off screen
        if (f22_to_float(system->asteroids[i].x) < -ASTEROID_BASE_SIZE) {
            system->asteroids[i].active = false;
        }
    }

    // Handle spawning
    system->spawn_timer += 1.0f/60.0f;
    if (system->spawn_timer >= 0.33f) {
        int num_layers = 0;//rand() % 3;  // 0 = none, 1 = one layer, 2 = two layers

        while (num_layers <= 3) {
            bool spawn_above = (rand() % 2) >= 1;  // 50% chance for above
            bool spawn_below = (rand() % 2) >= 1;  // 50% chance for below
            if (spawn_above) spawn_asteroid(system, wave, true, num_layers == 0 ? 1.0f: num_layers * 2.0f);
            if (!spawn_above) spawn_asteroid(system, wave, false, num_layers == 0 ? 1.0f: num_layers * 2.0f);
            num_layers++;
        }
        // if (num_layers >= 0) {
        //     // first layer
        //     bool spawn_above = (rand() % 2) >= 1;  // 50% chance for above
        //     bool spawn_below = (rand() % 2) >= 1;  // 50% chance for below
        //     if (spawn_above) spawn_asteroid(system, wave, true, 1.0f);
        //     if (!spawn_above) spawn_asteroid(system, wave, false, 1.0f);
        // }

        // if (num_layers >= 1) {
        //     // second layer, further out
        //     bool spawn_above = (rand() % 2) >= 1;
        //     bool spawn_below = (rand() % 2) >= 1;
        //     if (spawn_above) spawn_asteroid(system, wave, true, 2.5f);
        //     if (!spawn_above) spawn_asteroid(system, wave, false, 2.5f);
        // }

        system->spawn_timer = 0;
    }
}

void asteroid_system_render(const AsteroidSystem* system, SDL_Renderer* renderer, F22 camera_y_offset) {
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!system->asteroids[i].active) continue;

        // Transform points for rendering
        SDL_Point transformed[MAX_ASTEROID_POINTS];
        for (int j = 0; j < system->asteroids[i].num_points; j++) {
            // Rotate
            float angle = system->asteroids[i].rotation * M_PI / 180.0f;
            float cos_a = cosf(angle);
            float sin_a = sinf(angle);
            float px = system->asteroids[i].points[j].x;
            float py = system->asteroids[i].points[j].y;

            ScreenPos pos = world_to_screen(
                system->asteroids[i].x,
                system->asteroids[i].y,
                camera_y_offset
            );

            transformed[j].x = pos.x + (int)(px * cos_a - py * sin_a);
            transformed[j].y = pos.y + (int)(px * sin_a + py * cos_a);
            // transformed[j].x = (int)(f22_to_float(system->asteroids[i].x) +
            //                        (px * cos_a - py * sin_a));
            // transformed[j].y = (int)(f22_to_float(system->asteroids[i].y) +
            //                        (px * sin_a + py * cos_a));
        }

        // Draw the asteroid
        SDL_RenderDrawLines(renderer, transformed, system->asteroids[i].num_points);
    }
}

bool asteroid_system_check_collision(const AsteroidSystem* system, const Player* player) {
    const float PLAYER_RADIUS = 15.0f;  // match with game_state collision radius

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!system->asteroids[i].active) continue;

        // Simple circle collision for now
        float dx = f22_to_float(player->position.x) - f22_to_float(system->asteroids[i].x);
        float dy = f22_to_float(player->position.y) - f22_to_float(system->asteroids[i].y);
        float distance = sqrtf(dx * dx + dy * dy);

        if (distance < PLAYER_RADIUS + (ASTEROID_BASE_SIZE * system->asteroids[i].scale * 0.5f)) {
            return true;
        }
    }

    return false;
}
