#include "wave.h"
#include "config.h"
#include "f22.h"
#include "game_state.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL.h>

// WaveGenerator wave_init(void) {
//     WaveGenerator wave = {
//         .num_points = 0,
//         .current_segment = 0,
//         .last_x = f22_from_float(0.0f),
//         .scroll_speed = f22_from_float(2.0f),
//         .gravity = f22_from_float(0.075f),
//         .thrust = f22_from_float(0.15f)
//     };

//     // Generate initial control points
//     float x = 0;
//     float y = SCREEN_HEIGHT / 2.0f;  // Start in middle

//     for (int i = 0; i < 4; i++) {
//         wave.points[i].x = f22_from_float(x);
//         wave.points[i].y = f22_from_float(y);

//         // Generate next point
//         float segment_length = MIN_SEGMENT_LENGTH +
//             (rand() % (MAX_SEGMENT_LENGTH - MIN_SEGMENT_LENGTH));
//         x += segment_length;

//         // Calculate max possible y change based on physics
//         float time_in_segment = segment_length / f22_to_float(wave.scroll_speed);
//         float max_height_change = 0.5f * f22_to_float(wave.thrust) * time_in_segment * time_in_segment;

//         // Random y position within physically possible bounds
//         float y_change = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * max_height_change;
//         y = fmaxf(100.0f, fminf(SCREEN_HEIGHT - 100.0f, y + y_change));

//         wave.num_points++;
//     }

//     return wave;
// }
WaveGenerator wave_init(void) {
    WaveGenerator wave = {
        .num_points = GHOST_WIDTH,
        .last_x = f22_from_float(0.0f),
        .scroll_speed = SCROLL_SPEED,
        .position_offset = 0.0f,
        .ghost = {
            .x = f22_from_float(GHOST_WIDTH - 1),
            .y = f22_from_float(WINDOW_HEIGHT / 2),
            .velocity_y = f22_from_float(0.0f),
            .should_thrust = false,
            .optimal_height = WINDOW_HEIGHT / 2,
            .current_duration = 0.5f + ((float)rand() / (float)RAND_MAX) * 1.0f,
            .is_rest_phase = false,
            .elapsed_time = 0.0f,
            .phase_start_time = SDL_GetTicks() / 1000.0f
        }
    };

    for (int i = 0; i < GHOST_WIDTH; i++) {
        wave.points[i].x = f22_from_float(i);
        wave.points[i].y = f22_from_float(WINDOW_HEIGHT / 2);
        wave.points[i].activated = false;
    }
    return wave;
}

void wave_update_ghost(GhostPlayer* ghost, int player_y, float delta_time) {
    // Simple pattern: thrust for 30 frames, then rest for 30 frames
    float current_time = SDL_GetTicks() / 1000.0f;
    float diff = current_time - ghost->phase_start_time;
    if (diff >= ghost->current_duration) {
        ghost->phase_start_time = current_time;

        printf("SWITCHING FROM %f WITH CURRENT TIME %f WITH DIFF %f", f22_to_float(ghost->y), ghost->current_duration, diff);
        printf("CURRENT GHOST Y %f", f22_to_float(ghost->y));
        ghost->elapsed_time = 0.0f;
        if ((int)f22_to_float(ghost->y) <= player_y) {
            ghost->is_rest_phase = true;
        } else if ((int)f22_to_float(ghost->y) > player_y) {
            ghost->is_rest_phase = false;
        }
        // ghost->is_rest_phase = !ghost->is_rest_phase;

        if (ghost->is_rest_phase) {
            ghost->current_duration = ((float)rand() / (float)RAND_MAX) * 1.5f;
        } else {
            ghost->current_duration = ((float)rand() / (float)RAND_MAX) * 1.5f;
        }
    }

    ghost->should_thrust = !ghost->is_rest_phase;

    // Apply same physics as player
    // ghost->velocity_y = f22_add(ghost->velocity_y, f22_mul(GRAVITY, f22_from_float(delta_time)));
    // if (ghost->should_thrust) {
    //     ghost->velocity_y = f22_sub(ghost->velocity_y, f22_mul(THRUST, f22_from_float(delta_time)));
    // }
    ghost->velocity_y = f22_add(ghost->velocity_y, GRAVITY);
    if (ghost->should_thrust) {
        ghost->velocity_y = f22_sub(ghost->velocity_y, THRUST);
    }

    float current_vel = f22_to_float(ghost->velocity_y);
    if (current_vel > f22_to_float(MAX_VELOCITY)) {
        ghost->velocity_y = MAX_VELOCITY;
    } else if (current_vel < f22_to_float(MIN_VELOCITY)) {
        ghost->velocity_y = MIN_VELOCITY;
    }

    ghost->y = f22_add(ghost->y, ghost->velocity_y);
}

void wave_generate_next_point(WaveGenerator* wave) {
    // if (wave->num_points >= MAX_CONTROL_POINTS) {
    //     // Shift points left
    //     for (int i = 0; i < wave->num_points - 1; i++) {
    //         wave->points[i] = wave->points[i + 1];
    //     }
    //     wave->num_points--;
    // }

    // // Get last point
    // WavePoint last = wave->points[wave->num_points - 1];
    // WavePoint new_point;

    // // Generate next x position
    // float segment_length = MIN_SEGMENT_LENGTH +
    //     (rand() % (MAX_SEGMENT_LENGTH - MIN_SEGMENT_LENGTH));
    // new_point.x = f22_add(last.x, f22_from_float(segment_length));

    // // Calculate physically possible y change
    // float time_in_segment = segment_length / f22_to_float(wave->scroll_speed);
    // float max_height_change = 0.5f * f22_to_float(wave->thrust) * time_in_segment * time_in_segment;
    // max_height_change *= 10.0f;

    // // Generate new y within possible bounds
    // float last_y = f22_to_float(last.y);
    // float y_change = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * max_height_change;
    // float new_y = fmaxf(100.0f, fminf(SCREEN_HEIGHT - 100.0f, last_y + y_change));
    // new_point.y = f22_from_float(new_y);

    // wave->points[wave->num_points] = new_point;
    // wave->num_points++;
}

F22 wave_get_y_at_x(const WaveGenerator* wave, F22 x) {
    // Find segment containing x
    int segment = 0;
    for (int i = 0; i < wave->num_points - 1; i++) {
        if (f22_to_float(x) >= f22_to_float(wave->points[i].x) &&
            f22_to_float(x) < f22_to_float(wave->points[i + 1].x)) {
            segment = i;
            break;
        }
    }

    // Cubic interpolation between points
    float t = (f22_to_float(x) - f22_to_float(wave->points[segment].x)) /
              (f22_to_float(wave->points[segment + 1].x) - f22_to_float(wave->points[segment].x));

    float y0 = f22_to_float(wave->points[segment].y);
    float y1 = f22_to_float(wave->points[segment + 1].y);

    // Calculate control points for smooth curve
    float dx = f22_to_float(wave->points[segment + 1].x) - f22_to_float(wave->points[segment].x);
    float tension = 0.5f;  // Adjust for smoother/sharper curves

    float m0 = segment > 0 ?
        tension * (y1 - f22_to_float(wave->points[segment - 1].y)) :
        tension * (y1 - y0);

    float m1 = segment < wave->num_points - 2 ?
        tension * (f22_to_float(wave->points[segment + 2].y) - y0) :
        tension * (y1 - y0);

    // Hermite interpolation
    float t2 = t * t;
    float t3 = t2 * t;
    float h1 = 2*t3 - 3*t2 + 1;
    float h2 = -2*t3 + 3*t2;
    float h3 = t3 - 2*t2 + t;
    float h4 = t3 - t2;

    float y = h1*y0 + h2*y1 + h3*m0*dx + h4*m1*dx;
    return f22_from_float(y);
}

// void wave_update(WaveGenerator* wave) {
//     // Generate new points if needed
//     while (f22_to_float(wave->points[wave->num_points - 2].x) -
//            f22_to_float(wave->last_x) < GHOST_WIDTH * 1.5f) {
//         wave_generate_next_point(wave);
//     }

//     // Update scroll position
//     wave->last_x = f22_add(wave->last_x, wave->scroll_speed);

//     // Remove old points
//     while (wave->num_points > 6 &&
//            f22_to_float(wave->points[1].x) < f22_to_float(wave->last_x)) {
//         for (int i = 0; i < wave->num_points - 1; i++) {
//             wave->points[i] = wave->points[i + 1];
//         }
//         wave->num_points--;
//     }
// }
void wave_update(WaveGenerator* wave, int player_y, GameStateEnum state, float delta_time) {

    if (state == GAME_STATE_PLAYING) {
        wave_update_ghost(&wave->ghost, player_y, delta_time);
    } else {
        wave->ghost.y = f22_from_float(WINDOW_HEIGHT / 2);
        return;
    }

    // Use scroll speed to determine x position shift
    int shift = (int) wave->scroll_speed;// * fmaxf(delta_time, 0.0001f);

    // for (int i = 0; i < GHOST_WIDTH - 1; i++) {
    //     wave->points[i].x = f22_from_float(i);
    //     wave->points[i].y = wave->points[i + 1].y;
    // }
    // Shift existing points left
    int iterations = (int) GHOST_WIDTH / (int) shift;
    int i = 0;
    while (i < iterations)
    {
        int k = 0;
        while (k < (int) shift)
        {
            int index = i * (int) shift + k;
            wave->points[index].x = f22_from_float(index);
            wave->points[index].y = wave->points[index+(int)shift].y;
            wave->points[index].activated = wave->points[index+(int)shift].activated;
            k++;
        }
        i++;
    }

    int index = GHOST_WIDTH - shift;
    while (index < GHOST_WIDTH)
    {
        wave->points[index].x = f22_from_float(index);
        wave->points[index].y = wave->ghost.y;
        wave->points[index].activated = true;
        index++;
    }
    // wave_update_ghost(&wave->ghost);
    // for (int i = 0; i < GHOST_WIDTH - 1; i++) {
    //     wave->points[i].x = f22_from_float(i);
    //     wave->points[i].y = wave->points[i + 1].y;
    // }

    // // Add new point at ghost's position
    // wave->points[GHOST_WIDTH - 1].x = f22_from_float(GHOST_WIDTH - 1);
    // wave->points[GHOST_WIDTH - 1].y = wave->ghost.y;
}
