#include "asteroid.h"
#include "renderer.h"
#include <math.h>

#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)

static const SDL_Point ASTEROID_SHAPE[22] = {
    {20, -3},  {17, -8},  {19, -12}, {15, -17},  // top right chunk
    {10, -15}, {5, -18},  {0, -20},              // top edge
    {-7, -18}, {-12, -15},{-15, -10},            // top left chunk
    {-20, -5}, {-18, 0},  {-20, 5},              // left edge
    {-15, 10}, {-12, 15}, {-7, 18},              // bottom left
    {0, 20},   {7, 18},   {12, 15},              // bottom edge
    {17, 8},   {20, 3},   {20, -3}               // back to start
};

// Multiple craters and surface details
static const SDL_Point CRATER_DETAILS[] = {
    // Large semicircular crater top right
    {10, -13},  {10, -11}, {11, -10}, {14, -8}, {15, -8},

    // Original semicircular crater left
    {-14, 0},  {-13, -3}, {-10, -4}, {-7, -3},  {-6, 0},

    // Wide semicircular crater bottom
    {0, 12},   {-2, 10},  {-3, 8},   {-1, 6},   {1, 8},

    // Original small semicircular crater near center
    {3, -2},   {4, -3},   {5, -3},   {6, -2},   {7, 0},

    // New semicircular crater mid-left
    {-8, -6},  {-9, -8},  {-8, -10}, {-6, -9},  {-5, -7}
};

static void generate_asteroid(Asteroid* asteroid, float scale, F22 x, F22 y) {
    asteroid->x = x;
    asteroid->y = y;
    asteroid->scale = scale;
    asteroid->rotation = ((float)rand() / (float)RAND_MAX) * 360.0f;
    asteroid->rotation_speed = (((float)rand() / (float)RAND_MAX) * -1.0f);
    asteroid->active = true;
    asteroid->num_points = sizeof(ASTEROID_SHAPE) / sizeof(ASTEROID_SHAPE[0]);
    asteroid->num_crater_points = sizeof(CRATER_DETAILS) / sizeof(CRATER_DETAILS[0]);

    // Copy and scale base shape
    for (int i = 0; i < asteroid->num_points; i++) {
        asteroid->points[i].x = ASTEROID_SHAPE[i].x * scale;
        asteroid->points[i].y = ASTEROID_SHAPE[i].y * scale;
    }

    // Copy and scale crater details
    for (int i = 0; i < asteroid->num_crater_points; i++) {
        asteroid->craters[i].x = CRATER_DETAILS[i].x * scale;
        asteroid->craters[i].y = CRATER_DETAILS[i].y * scale;
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
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (system->asteroids[i].active) {
            float dx = f22_to_float(system->asteroids[i].x) - WINDOW_WIDTH;
            if (fabsf(dx) < MIN_ASTEROID_SPACING) return;
        }
    }

    int x_offset = 10 + rand() % 90;
    int spawn_x = WINDOW_WIDTH + ASTEROID_BASE_SIZE + x_offset;

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
        system->asteroids[i].rotation += system->asteroids[i].rotation_speed;
        if (system->asteroids[i].rotation > 360.0f) system->asteroids[i].rotation -= 360.0f;
        else if (system->asteroids[i].rotation < 0.0f) system->asteroids[i].rotation += 360.0f;

        // Deactivate if off screen
        if (f22_to_float(system->asteroids[i].x) < -ASTEROID_BASE_SIZE) {
            system->asteroids[i].active = false;
        }
    }

    // Handle spawning
    system->spawn_timer += 1.0f/60.0f;
    if (system->spawn_timer >= 0.4f) {
        int num_layers = 0;//rand() % 3;  // 0 = none, 1 = one layer, 2 = two layers

        while (num_layers <= 3) {
            bool spawn_above = (rand() % 3) >= 1;  // 50% chance for above
            bool spawn_below = (rand() % 3) >= 1;  // 50% chance for below
            if (spawn_above) spawn_asteroid(system, wave, true, num_layers == 0 ? 1.0f: num_layers * 2.0f);
            if (spawn_below) spawn_asteroid(system, wave, false, num_layers == 0 ? 1.0f: num_layers * 2.0f);
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

        float angle = system->asteroids[i].rotation * M_PI / 180.0f;
        float cos_a = cosf(angle);
        float sin_a = sinf(angle);

        ScreenPos pos = world_to_screen(
            system->asteroids[i].x,
            system->asteroids[i].y,
            camera_y_offset
        );

        // Draw main asteroid shape
        SDL_Point transformed_outline[32];  // match the increased array size
        for (int j = 0; j < system->asteroids[i].num_points; j++) {
            float px = system->asteroids[i].points[j].x;
            float py = system->asteroids[i].points[j].y;

            transformed_outline[j].x = pos.x + (int)(px * cos_a - py * sin_a);
            transformed_outline[j].y = pos.y + (int)(px * sin_a + py * cos_a);
        }

        // Draw the outline (closed shape)
        SDL_RenderDrawLines(renderer, transformed_outline, system->asteroids[i].num_points);
        // Connect last point to first to close the shape
        SDL_RenderDrawLine(renderer,
            transformed_outline[system->asteroids[i].num_points-1].x,
            transformed_outline[system->asteroids[i].num_points-1].y,
            transformed_outline[0].x,
            transformed_outline[0].y);

        // Draw crater details
        for (int j = 0; j < system->asteroids[i].num_crater_points; j += 5) {
            SDL_Point crater[5];
            for (int k = 0; k < 5; k++) {
                float px = system->asteroids[i].craters[j + k].x;
                float py = system->asteroids[i].craters[j + k].y;

                crater[k].x = pos.x + (int)(px * cos_a - py * sin_a);
                crater[k].y = pos.y + (int)(px * sin_a + py * cos_a);
            }
            // Draw the complete crater shape
            SDL_RenderDrawLines(renderer, crater, 5);
            // Close the crater shape
            // SDL_RenderDrawLine(renderer,
            //     crater[4].x, crater[4].y,
            //     crater[0].x, crater[0].y);
        }

        // Draw surface details (lines)
        // for (int j = system->asteroids[i].num_crater_points - 7; j < system->asteroids[i].num_crater_points; j += 2) {
        //     SDL_Point detail[2];
        //     for (int k = 0; k < 2; k++) {
        //         float px = system->asteroids[i].craters[j + k].x;
        //         float py = system->asteroids[i].craters[j + k].y;

        //         detail[k].x = pos.x + (int)(px * cos_a - py * sin_a);
        //         detail[k].y = pos.y + (int)(px * sin_a + py * cos_a);
        //     }
        //     SDL_RenderDrawLine(renderer, detail[0].x, detail[0].y, detail[1].x, detail[1].y);
        // }
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
