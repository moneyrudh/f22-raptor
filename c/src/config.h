#ifndef CONFIG_H
#define CONFIG_H

// Window dimensions
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Game physics constants
#define SCROLL_SPEED f22_from_float(2.0f)
#define GRAVITY f22_from_float(0.15f)
#define THRUST f22_from_float(0.25f)
#define MAX_ALTITUDE f22_from_float(100.0f)
#define MIN_ALTITUDE f22_from_float(0.0f)

// Other shared constants
#define OBSTACLE_WIDTH 50
#define OBSTACLE_GAP 200
#define MAX_OBSTACLES 5

#endif // CONFIG_H
