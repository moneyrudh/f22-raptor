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
} WavePoint;

typedef struct {
    F22 x;
    F22 y;
    F22 velocity_y;
    bool should_thrust;
    int thrust_counter;
} GhostPlayer;

typedef struct {
    WavePoint points[MAX_CONTROL_POINTS];
    int num_points;
    int current_segment;
    F22 last_x;
    F22 scroll_speed;
    // Physics constants matching player
    F22 gravity;
    F22 thrust;
} WaveGenerator;

// Function declarations only
WaveGenerator wave_init(void);
void wave_generate_next_point(WaveGenerator* wave);
F22 wave_get_y_at_x(const WaveGenerator* wave, F22 x);
void wave_update(WaveGenerator* wave);

#endif // WAVE_H
