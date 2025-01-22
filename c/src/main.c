#include <stdbool.h>
#include <SDL.h>
#include "game_state.h"
#include "renderer.h"
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Global state for emscripten main loop
typedef struct {
    bool quit;
    bool thrust_active;
    GameState game_state;
    Renderer renderer;
} GameContext;

void handle_input(GameContext* ctx) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                ctx->quit = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                game_state_handle_click(&ctx->game_state, event.button.x, event.button.y);
                #ifdef __EMSCRIPTEN__
                EM_ASM(
                    Module.setGameState(1);
                    console.log("GAME HAS BEGUN");
                );
                #endif
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                    case SDLK_UP:
                        ctx->thrust_active = true;
                        break;
                    case SDLK_ESCAPE:
                        ctx->quit = true;
                        break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                    case SDLK_UP:
                        ctx->thrust_active = false;
                        break;
                }
                break;
        }
    }
}

void main_loop(void* arg) {
    GameContext* ctx = (GameContext*)arg;

    handle_input(ctx);

    // Update game state
    game_state_update(&ctx->game_state, ctx->thrust_active);

    // Check collisions
    if (game_state_check_collisions(&ctx->game_state)) {
        #ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
        EM_ASM(
            console.log('Game Over! Score: ' + $0),
            ctx->game_state.score
        );
        #else
        ctx->quit = true;
        printf("Game Over! Score: %u\n", ctx->game_state.score);
        #endif
        return;
    }

    // Render frame
    renderer_draw_frame(&ctx->renderer, &ctx->game_state, ctx->thrust_active);

    #ifdef __EMSCRIPTEN__
    // SDL_Delay(16); // ~60 FPS cap for native builds
    #endif
}

int main() {
    #ifdef __EMSCRIPTEN__
    setvbuf(stdout, NULL, _IOLBF, 0);
    #endif

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return 1;
    }

    GameContext ctx = {
        .quit = false,
        .thrust_active = false,
        .game_state = game_state_init()
    };

    if (renderer_init(&ctx.renderer) < 0) {
        SDL_Log("Renderer init failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Rect viewport;
    SDL_RenderGetViewport(ctx.renderer.renderer, &viewport);
    printf("Viewport size: x=%d, y=%d, w=%d, h=%d\n",
           viewport.x, viewport.y, viewport.w, viewport.h);

    // Force the viewport size
    SDL_RenderSetLogicalSize(ctx.renderer.renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    #ifdef EMSCRIPTEN
        // Set up persistent 60 FPS main loop for web
        emscripten_set_main_loop_arg(main_loop, &ctx, 0, 1);
        setvbuf(stdout, NULL, _IOLBF, 0);
    #else
    // Regular main loop for native builds
    while (!ctx.quit) {
        main_loop(&ctx);
    }
    #endif

    renderer_cleanup(&ctx.renderer);
    SDL_Quit();
    return 0;
}
