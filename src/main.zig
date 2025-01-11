const std = @import("std");
const c = @cImport({
    @cInclude("SDL2/SDL.h");
});
// const F22 = @import("f22.zig").F22;
const GameState = @import("game_state").GameState;
const OBSTACLE_WIDTH = @import("game_state").OBSTACLE_WIDTH;
const WINDOW_WIDTH = @import("game_state").WINDOW_WIDTH;
const WINDOW_HEIGHT = @import("game_state").WINDOW_HEIGHT;

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

    // var player = Player.init();
    var game_state = GameState.init();
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
        game_state.update(thrust_active);

        if (game_state.checkCollisions()) {
            std.debug.print("Game Over! Score: {}\n", .{game_state.score});
            quit = true;
            continue;
        }

        // Clear screen
        _ = c.SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        _ = c.SDL_RenderClear(renderer);

        // _ = c.SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        // for (&game_state.obstacles) |*obstacle| {
        //     if (!obstacle.active) continue;

        //     const pos = obstacle.getScreenPosition();
        //     // Draw top obstacle
        //     var top_rect = c.SDL_Rect{
        //         .x = pos.x,
        //         .y = 0,
        //         .w = OBSTACLE_WIDTH,
        //         .h = pos.gap_y - 100,
        //     };
        //     _ = c.SDL_RenderFillRect(renderer, &top_rect);

        //     // Draw bottom obstacle
        //     var bottom_rect = c.SDL_Rect{
        //         .x = pos.x,
        //         .y = pos.gap_y + 100,
        //         .w = OBSTACLE_WIDTH,
        //         .h = WINDOW_HEIGHT - (pos.gap_y + 100),
        //     };
        //     _ = c.SDL_RenderFillRect(renderer, &bottom_rect);
        // }

        // Draw player (improved F-22 shape)
        const pos = game_state.player.getScreenPosition();

        // Enhanced thrust effect when active
        if (thrust_active) {
            const thrust = [27]c.SDL_Point{
                // Inner flame core (bright orange)
                .{ .x = pos.x - 60, .y = pos.y + 2 },
                .{ .x = pos.x - 64, .y = pos.y + 4 },
                .{ .x = pos.x - 85, .y = pos.y + 2 },
                .{ .x = pos.x - 64, .y = pos.y + 4 },
                .{ .x = pos.x - 85, .y = pos.y + 6 },
                .{ .x = pos.x - 64, .y = pos.y + 4 },
                .{ .x = pos.x - 60, .y = pos.y + 6 },

                // Middle flame layer (orange)
                .{ .x = pos.x - 60, .y = pos.y },
                .{ .x = pos.x - 95, .y = pos.y + 4 },
                .{ .x = pos.x - 100, .y = pos.y + 4 },
                .{ .x = pos.x - 95, .y = pos.y + 4 },
                .{ .x = pos.x - 60, .y = pos.y + 8 },

                // Outer flame effect (yellow)
                .{ .x = pos.x - 65, .y = pos.y + 4 },
                .{ .x = pos.x - 60, .y = pos.y - 2 },
                .{ .x = pos.x - 66, .y = pos.y - 3 },
                .{ .x = pos.x - 70, .y = pos.y - 3 },
                .{ .x = pos.x - 80, .y = pos.y - 2 },
                .{ .x = pos.x - 95, .y = pos.y + 2 },
                .{ .x = pos.x - 100, .y = pos.y + 4 },
                .{ .x = pos.x - 125, .y = pos.y + 4 },
                .{ .x = pos.x - 100, .y = pos.y + 4 },
                .{ .x = pos.x - 95, .y = pos.y + 6 },
                .{ .x = pos.x - 80, .y = pos.y + 10 },
                .{ .x = pos.x - 70, .y = pos.y + 11 },
                .{ .x = pos.x - 66, .y = pos.y + 11 },
                .{ .x = pos.x - 60, .y = pos.y + 10 },
                .{ .x = pos.x - 65, .y = pos.y + 4 },
            };

            // Draw inner flame (bright orange)
            _ = c.SDL_SetRenderDrawColor(renderer, 255, 100, 0, 255);
            _ = c.SDL_RenderDrawLines(renderer, thrust[0..7], 7);

            // Draw middle flame (orange)
            _ = c.SDL_SetRenderDrawColor(renderer, 255, 150, 50, 255);
            _ = c.SDL_RenderDrawLines(renderer, thrust[7..12], 5);

            // Draw outer flame (yellow)
            _ = c.SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
            _ = c.SDL_RenderDrawLines(renderer, thrust[12..], 15);
        }

        // Main F-22 shape with more detail
        const f22_shape = [32]c.SDL_Point{
            // Nose section
            .{ .x = pos.x + 50, .y = pos.y }, // nose tip
            .{ .x = pos.x + 45, .y = pos.y - 3 }, // nose tip
            .{ .x = pos.x + 40, .y = pos.y - 4 }, // upper nose
            .{ .x = pos.x + 32, .y = pos.y - 5 }, // canopy start
            .{ .x = pos.x + 30, .y = pos.y - 6 }, // canopy start
            // Cockpit section
            .{ .x = pos.x + 26, .y = pos.y - 10 }, // canopy peak
            .{ .x = pos.x + 23, .y = pos.y - 11 }, // canopy peak
            .{ .x = pos.x + 20, .y = pos.y - 12 }, // canopy peak
            .{ .x = pos.x + 16, .y = pos.y - 12 }, // canopy rear
            .{ .x = pos.x + 15, .y = pos.y - 11 }, // canopy rear
            .{ .x = pos.x + 14, .y = pos.y - 11 }, // canopy rear
            .{ .x = pos.x - 5, .y = pos.y - 6 }, // canopy rear
            // Main body to line
            .{ .x = pos.x - 38, .y = pos.y - 4 },
            // Vertical stabilizers (twin tails)
            .{ .x = pos.x - 45, .y = pos.y - 16 }, // left tail top
            .{ .x = pos.x - 53, .y = pos.y - 16 }, // left tail top
            .{ .x = pos.x - 54, .y = pos.y }, // left tail back
            // Main wing section
            .{ .x = pos.x + 22, .y = pos.y + 4 }, // wing root
            .{ .x = pos.x + 18, .y = pos.y + 8 }, // wing root
            .{ .x = pos.x, .y = pos.y + 8 }, // wing root
            .{ .x = pos.x - 38, .y = pos.y + 16 }, // wing tip
            .{ .x = pos.x - 48, .y = pos.y + 16 }, // wing trailing edge
            // .{ .x = pos.x - 48, .y = pos.y + 12 }, // wing trailing edge
            .{ .x = pos.x - 48, .y = pos.y + 8 }, // wing trailing edge
            // Rear fuselage
            .{ .x = pos.x - 40, .y = pos.y + 2 },
            .{ .x = pos.x - 60, .y = pos.y + 6 }, // tail end
            // Bottom line forward
            .{ .x = pos.x - 66, .y = pos.y + 4 },
            .{ .x = pos.x - 54, .y = pos.y },
            .{ .x = pos.x - 10, .y = pos.y + 2 },
            .{ .x = pos.x + 22, .y = pos.y + 4 },
            .{ .x = pos.x + 27, .y = pos.y + 4 },
            .{ .x = pos.x + 40, .y = pos.y + 3 },
            .{ .x = pos.x + 48, .y = pos.y + 1 },
            .{ .x = pos.x + 50, .y = pos.y }, // back to nose
        };

        _ = c.SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
        _ = c.SDL_RenderDrawLines(renderer, &f22_shape, 32);

        // Add surface detail lines
        const left_wing = [4]c.SDL_Point{
            .{ .x = pos.x - 5, .y = pos.y - 6 }, // wing root
            .{ .x = pos.x - 12, .y = pos.y - 12 }, // wing mid
            .{ .x = pos.x - 20, .y = pos.y - 12 }, // wing tip
            .{ .x = pos.x - 32, .y = pos.y - 4 }, // wing tip
        };
        _ = c.SDL_RenderDrawLines(renderer, &left_wing, 4);

        const left_tail = [6]c.SDL_Point{
            .{ .x = pos.x - 32, .y = pos.y - 4 }, // wing root
            .{ .x = pos.x - 35, .y = pos.y - 20 }, // wing mid
            .{ .x = pos.x - 41, .y = pos.y - 20 }, // wing tip
            .{ .x = pos.x - 43, .y = pos.y - 11 }, // wing tip
            .{ .x = pos.x - 38, .y = pos.y - 4 }, // wing tip
            .{ .x = pos.x - 32, .y = pos.y - 4 }, // wing tip
        };
        _ = c.SDL_RenderDrawLines(renderer, &left_tail, 6);

        const cock_pit = [5]c.SDL_Point{
            .{ .x = pos.x + 28, .y = pos.y - 8 },
            .{ .x = pos.x + 24, .y = pos.y - 6 },
            .{ .x = pos.x + 20, .y = pos.y - 6 },
            .{ .x = pos.x + 12, .y = pos.y - 7 },
            .{ .x = pos.x + 8, .y = pos.y - 9 },
        };
        _ = c.SDL_RenderDrawLines(renderer, &cock_pit, 5);

        // Draw score
        // (We'll add proper text rendering later)

        c.SDL_RenderPresent(renderer);
        c.SDL_Delay(16);
    }
}
