// sound.c
#include "sound.h"
#include <SDL.h>

SoundSystem sound_system_create(void) {
    SoundSystem system = {0};
    system.initialized = false;
    return system;
}

static int pick_next_track(int current) {
    int next;
    do {
        next = rand() % NUM_MUSIC_TRACKS;
    } while(next == current && NUM_MUSIC_TRACKS > 1);
    printf("NEXT: %d", next);
    return next;
}

static void play_random_track(SoundSystem* system) {
    char path[100];
    int next_track = pick_next_track(system->current_track);
    
    #ifdef __EMSCRIPTEN__
    snprintf(path, sizeof(path), "/assets/sounds/music/%d.mp3", next_track);
    #else  
    snprintf(path, sizeof(path), "assets/sounds/music/%d.mp3", next_track);
    #endif

    // Free previous music if any
    if (system->current_music) {
        Mix_FreeMusic(system->current_music);
    }

    system->current_music = Mix_LoadMUS(path);
    if (!system->current_music) {
        printf("Failed to load music track %d: %s\n", next_track, Mix_GetError());
        return;
    }
    printf("NOW PLAYING: %s", path);

    Mix_VolumeMusic(MUSIC_VOLUME);
    Mix_PlayMusic(system->current_music, 1);  // play once
    system->current_track = next_track;
}

static void music_finished_callback(void) {
    if (g_sound_system) {
        play_random_track(g_sound_system);
    }
}

void sound_system_init(SoundSystem* system) {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer init failed: %s\n", Mix_GetError());
        return;
    }

    system->current_track = -1;
    system->current_music = NULL;
    
    #ifdef __EMSCRIPTEN__
    system->f22_engine = Mix_LoadWAV("/assets/sounds/engine.mp3");
    system->collision = Mix_LoadWAV("/assets/sounds/collision.mp3");
    system->game_over = Mix_LoadWAV("/assets/sounds/game-over.mp3");
    #else
    system->f22_engine = Mix_LoadWAV("assets/sounds/engine.mp3");
    system->collision = Mix_LoadWAV("assets/sounds/collision.mp3");
    system->game_over = Mix_LoadWAV("/assets/sounds/game-over.mp3");
    #endif

    if (!system->f22_engine) {
        printf("Failed to load engine sound: %s\n", Mix_GetError());
    }
    if (!system->collision) {
        printf("Failed to load collision sound: %s\n", Mix_GetError());
    }
    if (!system->game_over) {
        printf("Failed to load game over sound: %s\n", Mix_GetError());
    }

    if (system->f22_engine) Mix_VolumeChunk(system->f22_engine, ENGINE_VOLUME);
    if (system->collision) Mix_VolumeChunk(system->collision, COLLISION_VOLUME);
    if (system->game_over) Mix_VolumeChunk(system->game_over, GAME_OVER_VOLUME);

    // Set up music finished callback
    Mix_HookMusicFinished(music_finished_callback);
    
    system->initialized = true;
    system->engine_channel = 0;
    Mix_ReserveChannels(1);
}

SoundSystem* get_sound_system() {
    return g_sound_system;
}

void set_sound_system(SoundSystem* system) {
    g_sound_system = system;
}

void play_random() {
    play_random_track(g_sound_system);
}

// void sound_system_init(SoundSystem* system) {
//     // init audio with good defaults for game sfx
//     if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
//         printf("SDL_mixer init failed: %s\n", Mix_GetError());
//         return;
//     }
    
//     // load our sounds
//     #ifdef __EMSCRIPTEN__
//     system->soundtrack = Mix_LoadMUS("/sounds/soundtrack.mp3");
//     system->f22_engine = Mix_LoadWAV("/sounds/engine.mp3");
//     system->collision = Mix_LoadWAV("/sounds/collision.mp3");
//     #else
//     system->soundtrack = Mix_LoadMUS("assets/sounds/soundtrack.mp3");
//     system->f22_engine = Mix_LoadWAV("assets/sounds/engine.mp3");
//     system->collision = Mix_LoadWAV("assets/sounds/collision.mp3");
//     #endif

//     // start background music looped
//     if (system->soundtrack) {
//         Mix_PlayMusic(system->soundtrack, -1);  // -1 = loop forever
//     }

//     // reserve channel 0 for engine sound
//     system->engine_channel = 0;
//     Mix_ReserveChannels(1);  // keep channel 0 just for engine
// }

void sound_system_cleanup(SoundSystem* system) {
    // first stop and free the music
    sound_system_stop_music(system);
    
    // then free sound effects
    if (system->f22_engine) {
        Mix_FreeChunk(system->f22_engine);
        system->f22_engine = NULL;
    }
    if (system->collision) {
        Mix_FreeChunk(system->collision);
        system->collision = NULL;
    }
    if (system->game_over) {
        Mix_FreeChunk(system->game_over);
        system->game_over = NULL;
    }
    
    // finally close audio system
    Mix_CloseAudio();
    system->initialized = false;
}

void sound_system_stop_music(SoundSystem* system) {
    if (system->current_music) {
        Mix_HaltMusic();  // stop any playing music
        Mix_FreeMusic(system->current_music);  // free the music data
        system->current_music = NULL;
        system->current_track = -1;
    }
}

void sound_system_start_engine(SoundSystem* system) {
    if (!system->engine_playing && system->f22_engine) {
        system->engine_playing = true;
        Mix_PlayChannel(system->engine_channel, system->f22_engine, -1);
    }
}

void sound_system_stop_engine(SoundSystem* system) {
    if (system->engine_playing) {
        Mix_HaltChannel(system->engine_channel);
        system->engine_playing = false;
    }
}

void sound_system_play_collision(SoundSystem* system) {
    if (system->collision) {
        Mix_PlayChannel(-1, system->collision, 0);  // play once on any free channel
    }
}

void sound_system_play_game_over(SoundSystem* system) {
    if (system->game_over) {
        Mix_PlayChannel(-1, system->game_over, 0);
    }
}