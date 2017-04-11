//
// Created by xiang on 2017-4-8.
//

#ifndef PLAYER_VIDEOCOMPONENT_H
#define PLAYER_VIDEOCOMPONENT_H

#include "VideoState.h"

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

/**
 * 设置SDL显示环境
 */
void alloc_picture(VideoState *is);

/**
 * 打开视频组件
 */
int video_stream_component_open(VideoState *is, int stream_index);

void schedule_refresh(VideoState *is, int delay);

void video_refresh_timer(VideoState *is);

void video_display(VideoState *is);

#endif //PLAYER_VIDEOCOMPONENT_H
