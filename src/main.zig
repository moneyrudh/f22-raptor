const std = @import("std");
const c = @cImport({
    @cInclude("SDL2/SDL.h");
});
const F22 = @import("f22.zig").F22;

// Game constants using F22 type
const GRAVITY = F22.fromFloat(-0.5);
const THRUST = F22.fromFloat(0.8);
const MAX_ALTITUDE = F22.fromFloat(100.0);
const MIN_ALTITUDE = F22.fromFloat(0.0);
const INITIAL_ALTITUDE = F22.fromFloat(22.0);

const WINDOW_WIDTH = 800;
const WINDOW_HEIGHT = 600;

const Player = struct {
    position: struct {
        x: F22,
        y: F22,
    },
    velocity: struct {
        x: F22,
        y: F22,
    },

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

pub fn main() !void {
    // Initialize SDL
    if (c.SDL_Init(c.SDL_INIT_VIDEO) < 0) {
        c.SDL_Log("Unable to initialize SDL: %s", c.SDL_GetError());
        return error.SDLInitializationFailed;
    }
    defer c.SDL_Quit();

    // Create window
    const window = c.SDL_CreateWindow(
        "F-22 Game",
        c.SDL_WINDOWPOS_CENTERED,
        c.SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        c.SDL_WINDOW_SHOWN,
    ) orelse {
        c.SDL_Log("Unable to create window: %s", c.SDL_GetError());
        return error.SDLInitializationFailed;
    };
    defer c.SDL_DestroyWindow(window);

    // Create renderer
    const renderer = c.SDL_CreateRenderer(window, -1, c.SDL_RENDERER_ACCELERATED) orelse {
        c.SDL_Log("Unable to create renderer: %s", c.SDL_GetError());
        return error.SDLInitializationFailed;
    };
    defer c.SDL_DestroyRenderer(renderer);

    var player = Player.init();
    var quit = false;
    var thrust_active = false;

    while (!quit) {
        var event: c.SDL_Event = undefined;
        while (c.SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                c.SDL_QUIT => {
                    quit = true;
                },
                c.SDL_KEYDOWN => {
                    switch (event.key.keysym.sym) {
                        c.SDLK_SPACE, c.SDLK_UP => {
                            thrust_active = true;
                        },
                        c.SDLK_ESCAPE => {
                            quit = true;
                        },
                        else => {},
                    }
                },
                c.SDL_KEYUP => {
                    switch (event.key.keysym.sym) {
                        c.SDLK_SPACE, c.SDLK_UP => {
                            thrust_active = false;
                        },
                        else => {},
                    }
                },
                else => {},
            }
        }

        // Update game state
        player.update(thrust_active);

        // Clear screen
        _ = c.SDL_SetRenderDrawColor(renderer, 25, 25, 50, 255);
        _ = c.SDL_RenderClear(renderer);

        // Draw player (simple triangle for now)
        const pos = player.getScreenPosition();
        const triangle = [4]c.SDL_Point{
            .{ .x = pos.x - 15, .y = pos.y + 10 }, // bottom left
            .{ .x = pos.x + 15, .y = pos.y + 10 }, // bottom right
            .{ .x = pos.x, .y = pos.y - 10 }, // top
            .{ .x = pos.x - 15, .y = pos.y + 10 }, // connect back to start
        };

        _ = c.SDL_SetRenderDrawColor(renderer, 180, 180, 230, 255);
        _ = c.SDL_RenderDrawLines(renderer, &triangle, 4);

        // If thrust is active, draw thrust flame
        if (thrust_active) {
            const thrust = [3]c.SDL_Point{
                .{ .x = pos.x - 8, .y = pos.y + 10 }, // left
                .{ .x = pos.x, .y = pos.y + 20 }, // bottom
                .{ .x = pos.x + 8, .y = pos.y + 10 }, // right
            };
            _ = c.SDL_SetRenderDrawColor(renderer, 255, 150, 50, 255);
            _ = c.SDL_RenderDrawLines(renderer, &thrust, 3);
        }

        c.SDL_RenderPresent(renderer);

        // Cap at ~60 FPS
        c.SDL_Delay(16);
    }
}
