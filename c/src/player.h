// player.h
#ifndef PLAYER_H
#define PLAYER_H

#include "f22.h"

typedef struct {
    F22 x;
    F22 y;
} Position;

typedef struct {
    Position position;
    Position velocity;
    float rotation;
} Player;

typedef enum {
    GAME_STATE_WAITING,
    GAME_STATE_PLAYING,
    GAME_STATE_OVER
} GameStateEnum;

#endif // PLAYER_H
