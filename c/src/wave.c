#include "wave.h"
#include <stdlib.h>
#include <math.h>

WaveGenerator wave_init(void) {
    WaveGenerator wave = {
        .num_points = 0,
        .current_segment = 0,
        .last_x = f22_from_float(0.0f),
        .scroll_speed = f22_from_float(2.0f),
        .gravity = f22_from_float(0.075f),
        .thrust = f22_from_float(0.15f)
    };

    // Generate initial control points
    float x = 0;
    float y = SCREEN_HEIGHT / 2.0f;  // Start in middle

    for (int i = 0; i < 4; i++) {
        wave.points[i].x = f22_from_float(x);
        wave.points[i].y = f22_from_float(y);

        // Generate next point
        float segment_length = MIN_SEGMENT_LENGTH +
            (rand() % (MAX_SEGMENT_LENGTH - MIN_SEGMENT_LENGTH));
        x += segment_length;

        // Calculate max possible y change based on physics
        float time_in_segment = segment_length / f22_to_float(wave.scroll_speed);
        float max_height_change = 0.5f * f22_to_float(wave.thrust) * time_in_segment * time_in_segment;

        // Random y position within physically possible bounds
        float y_change = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * max_height_change;
        y = fmaxf(100.0f, fminf(SCREEN_HEIGHT - 100.0f, y + y_change));

        wave.num_points++;
    }

    return wave;
}

void wave_generate_next_point(WaveGenerator* wave) {
    if (wave->num_points >= MAX_CONTROL_POINTS) {
        // Shift points left
        for (int i = 0; i < wave->num_points - 1; i++) {
            wave->points[i] = wave->points[i + 1];
        }
        wave->num_points--;
    }

    // Get last point
    WavePoint last = wave->points[wave->num_points - 1];
    WavePoint new_point;

    // Generate next x position
    float segment_length = MIN_SEGMENT_LENGTH +
        (rand() % (MAX_SEGMENT_LENGTH - MIN_SEGMENT_LENGTH));
    new_point.x = f22_add(last.x, f22_from_float(segment_length));

    // Calculate physically possible y change
    float time_in_segment = segment_length / f22_to_float(wave->scroll_speed);
    float max_height_change = 0.5f * f22_to_float(wave->thrust) * time_in_segment * time_in_segment;
    max_height_change *= 10.0f;

    // Generate new y within possible bounds
    float last_y = f22_to_float(last.y);
    float y_change = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * max_height_change;
    float new_y = fmaxf(100.0f, fminf(SCREEN_HEIGHT - 100.0f, last_y + y_change));
    new_point.y = f22_from_float(new_y);

    wave->points[wave->num_points] = new_point;
    wave->num_points++;
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

void wave_update(WaveGenerator* wave) {
    // Generate new points if needed
    while (f22_to_float(wave->points[wave->num_points - 2].x) -
           f22_to_float(wave->last_x) < WINDOW_WIDTH * 1.5f) {
        wave_generate_next_point(wave);
    }

    // Update scroll position
    wave->last_x = f22_add(wave->last_x, wave->scroll_speed);

    // Remove old points
    while (wave->num_points > 6 &&
           f22_to_float(wave->points[1].x) < f22_to_float(wave->last_x)) {
        for (int i = 0; i < wave->num_points - 1; i++) {
            wave->points[i] = wave->points[i + 1];
        }
        wave->num_points--;
    }
}
