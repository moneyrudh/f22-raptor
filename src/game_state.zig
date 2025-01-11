const std = @import("std");
const c = @cImport({
    @cInclude("SDL2/SDL.h");
});
const F22 = @import("f22.zig").F22;

// Additional game constants
pub const SCROLL_SPEED = F22.fromFloat(2.0);
pub const OBSTACLE_WIDTH = 50;
pub const OBSTACLE_GAP = 200;
pub const MAX_OBSTACLES = 5;
pub const INITIAL_ALTITUDE = F22.fromFloat(22.0);
pub const GRAVITY = F22.fromFloat(-0.05);
pub const THRUST = F22.fromFloat(0.1);
// Game constants using F22 type
pub const MAX_ALTITUDE = F22.fromFloat(100.0);
pub const MIN_ALTITUDE = F22.fromFloat(0.0);

pub const WINDOW_WIDTH = 800;
pub const WINDOW_HEIGHT = 600;

pub const Player = struct {
    position: struct {
        x: F22,
        y: F22,
    },
    velocity: struct {
        x: F22,
        y: F22,
    },
    rotation: f32,

    pub fn init() Player {
        return .{
            .position = .{
                .x = F22.fromFloat(50.0),
                .y = INITIAL_ALTITUDE,
            },
            .velocity = .{
                .x = F22.fromFloat(0.0),
                .y = F22.fromFloat(0.0),
            },
            .rotation = 0.0,
        };
    }

    pub fn update(self: *Player, thrust: bool) void {
        // Apply gravity
        self.velocity.y = self.velocity.y.add(GRAVITY);

        // Apply thrust if space/up is pressed
        if (thrust) {
            self.velocity.y = self.velocity.y.add(THRUST);
        }

        // Update position
        self.position.y = self.position.y.add(self.velocity.y);

        // Calculate rotation based on velocity
        const vel_y = self.velocity.y.toFloat();
        // Map velocity to rotation angle (-30 to 30 degrees)
        // Reduce the multiplier to make it less sensitive
        const target_rotation = vel_y * 50.0; // Reduced from 300.0
        // Smoothly interpolate current rotation to target
        // Use a smaller interpolation factor for smoother transitions
        self.rotation = (self.rotation - target_rotation) * 0.1; // Changed from 0.15
        // Clamp rotation to prevent extreme angles
        self.rotation = @max(-30.0, @min(30.0, self.rotation));

        // Clamp position to bounds
        if (self.position.y.toFloat() < MIN_ALTITUDE.toFloat()) {
            self.position.y = MIN_ALTITUDE;
            self.velocity.y = F22.fromFloat(0.0);
        } else if (self.position.y.toFloat() > MAX_ALTITUDE.toFloat()) {
            self.position.y = MAX_ALTITUDE;
            self.velocity.y = F22.fromFloat(0.0);
        }
    }

    // Convert game coordinates to screen coordinates
    pub fn getScreenPosition(self: *const Player) struct { x: i32, y: i32 } {
        return .{
            .x = @intFromFloat(self.position.x.toFloat() / 100.0 * @as(f32, WINDOW_WIDTH)),
            .y = @intFromFloat((1.0 - self.position.y.toFloat() / 100.0) * @as(f32, WINDOW_HEIGHT)),
        };
    }
};

pub const Obstacle = struct {
    x: F22,
    gap_y: F22,
    active: bool,

    pub fn init() Obstacle {
        return .{
            .x = F22.fromFloat(@as(f32, WINDOW_WIDTH)),
            .gap_y = F22.fromFloat(50.0), // Default gap position
            .active = false,
        };
    }

    pub fn update(self: *Obstacle) void {
        if (!self.active) return;
        self.x = self.x.sub(SCROLL_SPEED);

        // Deactivate if off screen
        if (self.x.toFloat() < -@as(f32, OBSTACLE_WIDTH)) {
            self.active = false;
        }
    }

    // Convert game coordinates to screen coordinates for rendering
    pub fn getScreenPosition(self: *const Obstacle) struct { x: i32, gap_y: i32 } {
        return .{
            .x = @intFromFloat(self.x.toFloat()),
            .gap_y = @intFromFloat((1.0 - self.gap_y.toFloat() / 100.0) * @as(f32, WINDOW_HEIGHT)),
        };
    }
};

pub const GameState = struct {
    player: Player,
    obstacles: [MAX_OBSTACLES]Obstacle,
    last_obstacle_x: F22,
    score: u32,

    pub fn init() GameState {
        var state = GameState{
            .player = Player.init(),
            .obstacles = undefined,
            .last_obstacle_x = F22.fromFloat(@as(f32, WINDOW_WIDTH)),
            .score = 0,
        };

        // Initialize obstacles
        for (&state.obstacles) |*obstacle| {
            obstacle.* = Obstacle.init();
        }

        return state;
    }

    pub fn update(self: *GameState, thrust_active: bool) void {
        // Update player
        self.player.update(thrust_active);

        // Update obstacles
        // for (&self.obstacles) |*obstacle| {
        //     if (obstacle.active) {
        //         obstacle.update();
        //     }
        // }

        // // Spawn new obstacles
        // if (self.last_obstacle_x.toFloat() - OBSTACLE_GAP >= 0) {
        //     if (self.findInactiveObstacle()) |obstacle| {
        //         obstacle.active = true;
        //         obstacle.x = F22.fromFloat(@as(f32, WINDOW_WIDTH));
        //         // Random gap position between 20% and 80% of screen height
        //         var random = std.crypto.random;
        //         const gap_y = random.float(f32) * 60.0 + 20.0;
        //         obstacle.gap_y = F22.fromFloat(gap_y);
        //         self.last_obstacle_x = obstacle.x;
        //     }
        // }

        // // Update last obstacle position
        // var min_x = F22.fromFloat(@as(f32, WINDOW_WIDTH));
        // for (&self.obstacles) |*obstacle| {
        //     if (obstacle.active and obstacle.x.toFloat() < min_x.toFloat()) {
        //         min_x = obstacle.x;
        //     }
        // }
        // self.last_obstacle_x = min_x;
    }

    fn findInactiveObstacle(self: *GameState) ?*Obstacle {
        for (&self.obstacles) |*obstacle| {
            if (!obstacle.active) {
                return obstacle;
            }
        }
        return null;
    }

    pub fn checkCollisions(self: *GameState) bool {
        const player_pos = self.player.getScreenPosition();
        const player_radius = 15; // Simplified collision using circular boundary

        for (&self.obstacles) |*obstacle| {
            if (!obstacle.active) continue;

            const obs_pos = obstacle.getScreenPosition();
            // Check collision with vertical barriers
            if (player_pos.x + player_radius > obs_pos.x and
                player_pos.x - player_radius < obs_pos.x + OBSTACLE_WIDTH)
            {
                // Check if player is outside the gap
                if (player_pos.y - player_radius < obs_pos.gap_y - 100 or
                    player_pos.y + player_radius > obs_pos.gap_y + 100)
                {
                    return true;
                }
            }
        }
        return false;
    }
};
