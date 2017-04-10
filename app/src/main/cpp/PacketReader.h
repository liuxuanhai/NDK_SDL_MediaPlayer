//
// Created by xiang on 2017-4-8.
//

#ifndef PLAYER_VIDEODECODE_H


#include "VideoState.h"

#define MAX_AUDIO_SIZE (5 * 16 * 1024)
#define MAX_VIDEO_SIZE (5 * 256 * 1024)

/**
 * 创建读包线程
 */
int startReadPktThread(VideoState *is);

#endif //PLAYER_VIDEODECODE_H
