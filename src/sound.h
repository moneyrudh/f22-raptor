// sound.h
#ifndef SOUND_H
#define SOUND_H

#include <SDL_mixer.h>
#include <stdbool.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define NUM_MUSIC_TRACKS 7
#define ENGINE_VOLUME 75     // 25% volume (0-128 range)
#define COLLISION_VOLUME 50
#define MUSIC_VOLUME 50
#define GAME_OVER_VOLUME 50

typedef struct {
    Mix_Music* current_music;
    Mix_Music* soundtrack;     // background music
    Mix_Chunk* f22_engine;     // engine loop
    Mix_Chunk* collision;      // crash sound
    Mix_Chunk* game_over;
    bool engine_playing;      // track if engine sound is currently playing
    int engine_channel;       // keep track of which channel plays engine
    bool initialized;
    int current_track;
    int sfx_volume;
    int music_volume;
} SoundSystem;

static SoundSystem* g_sound_system = NULL;

static void music_finished_callback(void);

// init with reasonable defaults
SoundSystem sound_system_create(void);
void sound_system_init(SoundSystem* system);

// main functions you'll need
void sound_system_cleanup(SoundSystem* system);
void sound_system_stop_music(SoundSystem* system);
void sound_system_start_engine(SoundSystem* system);
void sound_system_stop_engine(SoundSystem* system);
void sound_system_play_collision(SoundSystem* system);
void sound_system_play_game_over(SoundSystem* system);

SoundSystem* get_sound_system();
void set_sound_system(SoundSystem* system);
void play_random();

void setSFXVolume(int volume);
void setMusicVolume(int volume);

#endif