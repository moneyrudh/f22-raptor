#ifndef CONFIG_H
#define CONFIG_H

// Window dimensions
#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define GHOST_WIDTH 1700

// Game physics constants
#define SCROLL_SPEED 10
// #define GRAVITY f22_from_float(10.15f)
// #define THRUST f22_from_float(20.35f)
#define GRAVITY f22_from_float(0.15f)
#define THRUST f22_from_float(0.35f)
#define MAX_ALTITUDE f22_from_float(100.0f)
#define MIN_ALTITUDE f22_from_float(0.0f)

// Other shared constants
#define OBSTACLE_WIDTH 50
#define OBSTACLE_GAP 200
#define MAX_OBSTACLES 5

// Safe zone constants
#define SAFE_ZONE_WIDTH 100
#define SAFE_ZONE_X (WINDOW_WIDTH / 2)  // center of screen
#define MAX_HORIZONTAL_MOVEMENT 5.0f     // max pixels per frame to move left
#define GAME_OVER_X 50

// Game constants
#define OBSTACLE_WIDTH 50
#define OBSTACLE_GAP 200
#define MAX_OBSTACLES 5

#define WORLD_TO_SCREEN_SCALE 1.0f
#define MAX_VELOCITY f22_from_float(10.0f)  // adjust this value to feel right
#define MIN_VELOCITY f22_from_float(-10.0f)

#endif // CONFIG_H
