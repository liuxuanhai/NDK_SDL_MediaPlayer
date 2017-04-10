//
// Created by xiang on 2017-4-8.
//

#ifndef PLAYER_AUDIOCOMPONENT_H
#define PLAYER_AUDIOCOMPONENT_H

#include "VideoState.h"

//音频缓冲区大小
#define SDL_AUDIO_BUFFER_SIZE 1024

/**
 * 打开音频组件，设置VideoState的音频设置
 */
int audio_stream_component_open(VideoState *is, int stream_index);

#endif //PLAYER_AUDIOCOMPONENT_H
