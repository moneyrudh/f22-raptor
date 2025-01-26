// player.h
#ifndef PLAYER_H
#define PLAYER_H

#include "f22.h"
#include <SDL.h>
#include <math.h>

#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)

typedef struct {
    F22 x;
    F22 y;
} Position;

// Screen position helpers
typedef struct {
    int x;
    int y;
} ScreenPos;

typedef struct {
    Position position;
    Position velocity;
    float rotation;
} Player;

typedef enum {
    GAME_STATE_WAITING,
    GAME_STATE_PLAYING,
    GAME_STATE_OVER
} GameStateEnum;

typedef struct {
    float score;
    float current_precision;
    float score_rate;
} ScoringSystem;


typedef struct {
    int x, y;
} EdgePoint;

// Helper function to fill a polygon
static void fill_polygon(SDL_Renderer* renderer, SDL_Point* points, int num_points) {
    // Find min and max y coordinates to know where to scan
    int min_y = points[0].y;
    int max_y = points[0].y;
    for (int i = 1; i < num_points; i++) {
        min_y = min(min_y, points[i].y);
        max_y = max(max_y, points[i].y);
    }

    // For each scan line
    for (int y = min_y; y <= max_y; y++) {
        EdgePoint intersections[32];  // Store x coords where scan line intersects edges
        int num_intersections = 0;

        // Find intersections with all edges
        for (int i = 0; i < num_points; i++) {
            int j = (i + 1) % num_points;  // Next point (wraps around)
            
            // Skip horizontal lines
            if (points[i].y == points[j].y) continue;
            
            // Check if the scan line intersects this edge
            if ((points[i].y > y && points[j].y <= y) ||
                (points[j].y > y && points[i].y <= y)) {
                
                // Calculate x coordinate of intersection using linear interpolation
                int x = points[i].x + (points[j].x - points[i].x) * 
                        (y - points[i].y) / (points[j].y - points[i].y);
                
                intersections[num_intersections++].x = x;
            }
        }

        // Sort intersections by x coordinate
        for (int i = 0; i < num_intersections - 1; i++) {
            for (int j = 0; j < num_intersections - i - 1; j++) {
                if (intersections[j].x > intersections[j + 1].x) {
                    EdgePoint temp = intersections[j];
                    intersections[j] = intersections[j + 1];
                    intersections[j + 1] = temp;
                }
            }
        }

        // Draw horizontal lines between pairs of intersections
        for (int i = 0; i < num_intersections - 1; i += 2) {
            SDL_RenderDrawLine(renderer, 
                intersections[i].x, y,
                intersections[i + 1].x, y);
        }
    }
}

#endif // PLAYER_H
