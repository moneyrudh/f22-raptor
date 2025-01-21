#ifndef RENDERER_H
#define RENDERER_H

#include <SDL.h>
#include "game_state.h"

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Point f22_shape[32];     // Store F22 shape points
    SDL_Point thrust_shape[27];  // Store thrust effect points
    SDL_Point left_wing[4];
    SDL_Point left_tail[6];
    SDL_Point cock_pit[5];
    SDL_Point wave_points[WINDOW_WIDTH];
    int num_wave_points;
} Renderer;

// Core rendering functions
int renderer_init(Renderer* renderer);
void renderer_cleanup(Renderer* renderer);
void renderer_draw_frame(Renderer* renderer, const GameState* state, bool thrust_active);
void renderer_draw_wave(Renderer* renderer, const WaveGenerator* wave, F22 camera_offset);

// Helper functions
void renderer_init_shapes(Renderer* renderer);
void renderer_rotate_points(SDL_Point* points, int num_points, SDL_Point center, float angle);
void renderer_draw_player(Renderer* renderer, const Player* player, F22 camera_y_offset, bool thrust_active);
void renderer_draw_obstacles(Renderer* renderer, const Obstacle* obstacles);

#endif // RENDERER_H
