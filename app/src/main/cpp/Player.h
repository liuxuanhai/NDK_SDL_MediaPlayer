//
// Created by xiang on 2017-4-8.
//

#ifndef PLAYER_PLAYER_H
#define PLAYER_PLAYER_H

#include <stdint.h>

#define FF_ALLOC_EVENT (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)

typedef enum PLAY_STATE {
    PLAYING,
    PAUSE,
    ERROR,
    FINISH,
    QUIT
} PLAY_STATE;

PLAY_STATE playURI(char *filename);

#endif //PLAYER_PLAYER_H