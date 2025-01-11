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
        _ = c.SDL_SetRenderDrawColor(renderer, 25, 25, 50, 255);
        _ = c.SDL_RenderClear(renderer);

        _ = c.SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        for (&game_state.obstacles) |*obstacle| {
            if (!obstacle.active) continue;

            const pos = obstacle.getScreenPosition();
            // Draw top obstacle
            var top_rect = c.SDL_Rect{
                .x = pos.x,
                .y = 0,
                .w = OBSTACLE_WIDTH,
                .h = pos.gap_y - 100,
            };
            _ = c.SDL_RenderFillRect(renderer, &top_rect);

            // Draw bottom obstacle
            var bottom_rect = c.SDL_Rect{
                .x = pos.x,
                .y = pos.gap_y + 100,
                .w = OBSTACLE_WIDTH,
                .h = WINDOW_HEIGHT - (pos.gap_y + 100),
            };
            _ = c.SDL_RenderFillRect(renderer, &bottom_rect);
        }

        // Draw player (improved F-22 shape)
        const pos = game_state.player.getScreenPosition();
        const f22_shape = [7]c.SDL_Point{
            .{ .x = pos.x - 20, .y = pos.y + 5 }, // tail bottom
            .{ .x = pos.x - 15, .y = pos.y }, // tail mid
            .{ .x = pos.x - 10, .y = pos.y - 5 }, // tail top
            .{ .x = pos.x, .y = pos.y - 8 }, // cockpit
            .{ .x = pos.x + 15, .y = pos.y - 3 }, // nose
            .{ .x = pos.x + 10, .y = pos.y + 5 }, // bottom front
            .{ .x = pos.x - 20, .y = pos.y + 5 }, // back to tail
        };

        _ = c.SDL_SetRenderDrawColor(renderer, 180, 180, 230, 255);
        _ = c.SDL_RenderDrawLines(renderer, &f22_shape, 7);

        // Draw thrust (if active)
        if (thrust_active) {
            const thrust = [5]c.SDL_Point{
                .{ .x = pos.x - 20, .y = pos.y + 3 }, // top
                .{ .x = pos.x - 25, .y = pos.y + 5 }, // middle
                .{ .x = pos.x - 20, .y = pos.y + 7 }, // bottom
                .{ .x = pos.x - 28, .y = pos.y + 5 }, // flame tip
                .{ .x = pos.x - 20, .y = pos.y + 3 }, // back to top
            };
            _ = c.SDL_SetRenderDrawColor(renderer, 255, 150, 50, 255);
            _ = c.SDL_RenderDrawLines(renderer, &thrust, 5);
        }

        // Draw score
        // (We'll add proper text rendering later)

        c.SDL_RenderPresent(renderer);
        c.SDL_Delay(16);
    }
}
