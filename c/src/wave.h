#ifndef WAVE_H
#define WAVE_H

#include "f22.h"
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "config.h"

#define MAX_CONTROL_POINTS 128
#define SCREEN_HEIGHT 600
#define MIN_SEGMENT_LENGTH 200
#define MAX_SEGMENT_LENGTH 400

typedef struct {
    F22 x;
    F22 y;
    bool activated;
} WavePoint;

typedef struct {
    F22 x;
    F22 y;
    F22 velocity_y;
    bool should_thrust;
    float current_duration;
    int optimal_height;
    float elapsed_time;
    float phase_start_time;
    bool is_rest_phase;
} GhostPlayer;

// typedef struct {
//     WavePoint points[MAX_CONTROL_POINTS];
//     int num_points;
//     int current_segment;
//     F22 last_x;
//     F22 scroll_speed;
//     // Physics constants matching player
//     F22 gravity;
//     F22 thrust;
// } WaveGenerator;

typedef struct {
    WavePoint points[GHOST_WIDTH];
    int num_points;
    F22 last_x;
    int scroll_speed;
    float position_offset;
    GhostPlayer ghost;
} WaveGenerator;

// Function declarations only
WaveGenerator wave_init(void);
void wave_generate_next_point(WaveGenerator* wave);
F22 wave_get_y_at_x(const WaveGenerator* wave, F22 x);
void wave_update(WaveGenerator* wave, int player_y);

#endif // WAVE_H
