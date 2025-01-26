#ifndef RENDERER_H
#define RENDERER_H

#include <SDL.h>
// #include <SDL_ttf.h>
#include "game_state.h"

typedef struct {
    float x, y;      // Star position
    float size;      // Size of the star (0-2 pixels)
    float speed;     // Movement speed
    float hue;       // Color variation
    int brightness;
} Star;

typedef struct {
    Star stars[250];
    float gradient_opacity;
    int gradient_direction;
    uint32_t last_frame;
    SDL_Texture* star_texture;  // Add texture to store star layer
} Background;

typedef struct {
    float x, y;
    float alpha;
    float lifetime;
} WaveParticle;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    Background* background;
    SDL_Point f22_shape[32];     // Store F22 shape points
    SDL_Point thrust_shape[27];  // Store thrust effect points
    SDL_Point left_wing[4];
    SDL_Point left_tail[6];
    SDL_Point cock_pit[5];
    SDL_Point pilot_circle[16];
    SDL_Point wave_points[WINDOW_WIDTH];
    WaveParticle particles[1000];
    int num_particles;
    uint32_t last_particle_spawn;
    int num_wave_points;
} Renderer;

Background* background_init(SDL_Renderer* renderer);
void update_star_texture(SDL_Renderer* renderer, Background* bg);
void draw_background(SDL_Renderer* renderer, Background* bg, const GameState* const);
void DrawCircle(SDL_Renderer* renderer, int cx, int cy, int radius);

// Core rendering functions
int renderer_init(Renderer* renderer);
void renderer_cleanup(Renderer* renderer);
void renderer_draw_frame(Renderer* renderer, const GameState* state, bool thrust_active);
void renderer_draw_wave(Renderer* renderer, const WaveGenerator* wave, const Player* player, F22 camera_offset);

// Helper functions
void renderer_init_shapes(Renderer* renderer);
void renderer_rotate_points(SDL_Point* points, int num_points, SDL_Point center, float angle);
void renderer_draw_player(Renderer* renderer, const Player* player, F22 camera_y_offset, bool thrust_active);
void renderer_draw_obstacles(Renderer* renderer, const Obstacle* obstacles);

// void renderer_draw_text(Renderer* renderer, const char* text, int x, int y, SDL_Color color);
// int renderer_init_font(Renderer* renderer, const char* font_path, int font_size);
// void renderer_cleanup_font(Renderer* renderer);

#endif // RENDERER_H
