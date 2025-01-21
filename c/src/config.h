#ifndef CONFIG_H
#define CONFIG_H

// Window dimensions
#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 800

// Game physics constants
#define SCROLL_SPEED 10
#define GRAVITY f22_from_float(0.15f)
#define THRUST f22_from_float(0.35f)
#define MAX_ALTITUDE f22_from_float(100.0f)
#define MIN_ALTITUDE f22_from_float(0.0f)

// Other shared constants
#define OBSTACLE_WIDTH 50
#define OBSTACLE_GAP 200
#define MAX_OBSTACLES 5

#endif // CONFIG_H
