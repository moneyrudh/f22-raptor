#include "explosion.h"

// Main shapes for debris
static const SDL_Point WING_SHAPE[] = {
    {0, 0}, {18, 8}, {-20, 16}, {-38, 16}, {0, 0}
};

static const SDL_Point TAIL_SHAPE[] = {
    {-38, -4}, {-45, -16}, {-53, -16}, {-54, 0}, {-38, -4}
};

static const SDL_Point NOSE_SHAPE[] = {
    {50, 0}, {45, -3}, {40, -4}, {32, -5}, {50, 0}
};

static const SDL_Point CANOPY_SHAPE[] = {
    {32, -5}, {26, -10}, {16, -12}, {14, -11}, {32, -5}
};

ExplosionSystem explosion_init(void) {
    ExplosionSystem system = {0};
    system.active = false;
    return system;
}

void explosion_reset(ExplosionSystem* system) {
    // Reset main system state
    system->active = false;
    system->time = 0;
    system->origin_x = f22_from_float(0);
    system->origin_y = f22_from_float(0);
    
    // Clear all debris pieces
    for (int i = 0; i < MAX_DEBRIS; i++) {
        system->debris[i].active = false;
    }
    
    // Clear all sparks
    for (int i = 0; i < MAX_SPARKS; i++) {
        system->sparks[i].active = false;
    }
}

void create_debris_piece(Debris* debris, const SDL_Point* shape, int num_points, 
                        float x, float y, float base_vx, float spread) {
    debris->active = true;
    debris->lifetime = 0;
    debris->x = x;
    debris->y = y;
    
    // Copy shape points
    memcpy(debris->points, shape, num_points * sizeof(SDL_Point));
    debris->num_points = num_points;
    
    // Random velocity with spread
    float angle = ((float)rand() / (float)RAND_MAX) * 2 * M_PI;
    float speed = 2.0f + ((float)rand() / (float)RAND_MAX) * 4.0f;
    debris->vx = base_vx + cosf(angle) * speed * spread;
    debris->vy = sinf(angle) * speed * spread;
    
    // Random rotation
    debris->rotation = ((float)rand() / (float)RAND_MAX) * 360.0f;
    debris->rot_speed = -180.0f + ((float)rand() / (float)RAND_MAX) * 360.0f;
    
    // Random scale variation
    debris->scale = 0.8f + ((float)rand() / (float)RAND_MAX) * 0.4f;
    
    // Hot metal colors
    debris->r = 230 + ((float)rand() / (float)RAND_MAX) * 25;
    debris->g = 120 + ((float)rand() / (float)RAND_MAX) * 80;
    debris->b = 50 + ((float)rand() / (float)RAND_MAX) * 30;
}

void create_spark(Spark* spark, float x, float y, float base_vx) {
    spark->active = true;
    spark->lifetime = 0;
    spark->x = x;
    spark->y = y;
    
    float angle = ((float)rand() / (float)RAND_MAX) * 2 * M_PI;
    float speed = 1.0f + ((float)rand() / (float)RAND_MAX) * 6.0f;
    spark->vx = base_vx + cosf(angle) * speed;
    spark->vy = sinf(angle) * speed;
    
    // bright orange/yellow colors
    spark->r = 255;
    spark->g = 180 + ((float)rand() / (float)RAND_MAX) * 75;
    spark->b = ((float)rand() / (float)RAND_MAX) * 50;
    spark->a = 255;
}

void explosion_start(ExplosionSystem* system, const Player* player) {
    if (system->active) return;
    
    system->active = true;
    system->time = 0;
    system->origin_x = player->position.x;
    system->origin_y = player->position.y;
    
    float x = f22_to_float(player->position.x);
    float y = f22_to_float(player->position.y);
    float base_vx = -2.0f;  // initial leftward velocity
    
    // Create wing debris
    for (int i = 0; i < 8; i++) {
        create_debris_piece(&system->debris[i], WING_SHAPE, 5, x, y, base_vx, 1.0f);
    }
    
    // Create tail debris
    for (int i = 8; i < 16; i++) {
        create_debris_piece(&system->debris[i], TAIL_SHAPE, 5, x, y, base_vx, 1.2f);
    }
    
    // Create nose debris
    for (int i = 16; i < 24; i++) {
        create_debris_piece(&system->debris[i], NOSE_SHAPE, 5, x, y, base_vx, 0.8f);
    }
    
    // Create canopy debris
    for (int i = 24; i < 32; i++) {
        create_debris_piece(&system->debris[i], CANOPY_SHAPE, 5, x, y, base_vx, 1.1f);
    }
    
    // Create smaller random debris
    for (int i = 32; i < MAX_DEBRIS; i++) {
        SDL_Point small_shape[] = {
            {0, 0}, 
            {((float)rand() / (float)RAND_MAX) * 10, ((float)rand() / (float)RAND_MAX) * 10},
            {((float)rand() / (float)RAND_MAX) * 10, ((float)rand() / (float)RAND_MAX) * -10},
            {0, 0}
        };
        create_debris_piece(&system->debris[i], small_shape, 4, x, y, base_vx, 1.5f);
    }
    
    // Create initial spark burst
    for (int i = 0; i < MAX_SPARKS; i++) {
        create_spark(&system->sparks[i], x, y, base_vx);
    }
}

void explosion_update(ExplosionSystem* system, float delta_time) {
    if (!system->active) return;
    
    system->time += delta_time;
    if (system->time >= EXPLOSION_DURATION) {
        system->active = false;
        return;
    }
    
    const float DRAG = 0.99f;
    
    // Update debris
    for (int i = 0; i < MAX_DEBRIS; i++) {
        Debris* d = &system->debris[i];
        if (!d->active) continue;
        
        d->lifetime += delta_time;
        if (d->lifetime > EXPLOSION_DURATION) {
            d->active = false;
            continue;
        }
        
        // Update position
        d->x += d->vx;
        d->y += d->vy;
        
        // Apply gravity and drag
        d->vy += f22_to_float(GRAVITY);
        d->vx *= DRAG;
        d->vy *= DRAG;
        
        // Update rotation
        d->rotation += d->rot_speed * delta_time;
        d->rot_speed *= 0.98f;  // slow rotation over time
    }
    
    // Update sparks
    for (int i = 0; i < MAX_SPARKS; i++) {
        Spark* s = &system->sparks[i];
        if (!s->active) continue;
        
        s->lifetime += delta_time;
        if (s->lifetime > 0.5f) {  // sparks are shorter-lived
            s->active = false;
            continue;
        }
        
        s->x += s->vx;
        s->y += s->vy;
        s->vy += f22_to_float(GRAVITY) * 0.5f;  // lighter gravity for sparks
        s->vx *= DRAG;
        s->vy *= DRAG;
        
        // Fade out
        float fade = 1.0f - (s->lifetime / 0.5f);
        s->a = (uint8_t)(255.0f * fade);
        
        // Occasionally spawn new sparks
        if (((float)rand() / (float)RAND_MAX) < 0.1f) {
            create_spark(&system->sparks[rand() % MAX_SPARKS], s->x, s->y, s->vx * 0.5f);
        }
    }
}

ScreenPos _world_to_screen(F22 world_x, F22 world_y, F22 camera_y_offset) {
    ScreenPos pos = {
        .x = (int)(f22_to_float(world_x) * WORLD_TO_SCREEN_SCALE),
        .y = (int)((f22_to_float(world_y) - f22_to_float(camera_y_offset)) * WORLD_TO_SCREEN_SCALE)
    };
    return pos;
}


void explosion_render(const ExplosionSystem* system, SDL_Renderer* renderer, F22 camera_y_offset) {
    if (!system->active) return;
    
    // First render debris
    for (int i = 0; i < MAX_DEBRIS; i++) {
        const Debris* d = &system->debris[i];
        if (!d->active) continue;
        
        // Transform points
        SDL_Point transformed[8];
        float cos_rot = cosf(d->rotation * M_PI / 180.0f);
        float sin_rot = sinf(d->rotation * M_PI / 180.0f);
        
        ScreenPos pos = _world_to_screen(
            f22_from_float(d->x), 
            f22_from_float(d->y), 
            camera_y_offset
        );
        
        for (int j = 0; j < d->num_points; j++) {
            float px = d->points[j].x * d->scale;
            float py = d->points[j].y * d->scale;
            
            transformed[j].x = pos.x + (int)(px * cos_rot - py * sin_rot);
            transformed[j].y = pos.y + (int)(px * sin_rot + py * cos_rot);
        }
        
        // Draw debris piece
        SDL_SetRenderDrawColor(renderer, d->r, d->g, d->b, 255);
        SDL_RenderDrawLines(renderer, transformed, d->num_points);
    }
    
    // Then render sparks on top
    for (int i = 0; i < MAX_SPARKS; i++) {
        const Spark* s = &system->sparks[i];
        if (!s->active) continue;
        
        ScreenPos pos = _world_to_screen(
            f22_from_float(s->x),
            f22_from_float(s->y),
            camera_y_offset
        );
        
        // Draw spark as small lines with glow effect
        SDL_SetRenderDrawColor(renderer, s->r, s->g, s->b, s->a);
        SDL_RenderDrawLine(renderer, 
            pos.x - 1, pos.y - 1,
            pos.x + 1, pos.y + 1
        );
        SDL_RenderDrawLine(renderer,
            pos.x - 1, pos.y + 1,
            pos.x + 1, pos.y - 1
        );
    }
}