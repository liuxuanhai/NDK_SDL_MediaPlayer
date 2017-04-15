//
// Created by xiang on 2017-4-8.
//

#include <SDL_events.h>
#include "Player.h"
#include "PacketReader.h"
#include "VideoComponent.h"
#include "LOG.h"

void onKeyDown(SDL_Keycode keycode, VideoState *is) {
    SDL_Event event;
    switch (keycode) {
        case SDLK_AC_BACK:  //如果按下返回键，则发送退出消息
            event.type = SDL_QUIT;
            event.user.data1 = is;
            SDL_PushEvent(&event);
            break;
    }
}

void onHandMessage(SDL_Event *event, VideoState *is) {
    switch (event->type) {
        case FF_ALLOC_EVENT:    //处理SDL创建界面
            alloc_picture(event->user.data1);
            break;
        case FF_REFRESH_EVENT:   //更新画面
            video_refresh_timer(event->user.data1);
            break;
        case SDL_QUIT:
            if (is->play_state == PLAYING) is->play_state = QUIT;
            break;
        case SDL_KEYDOWN:
            onKeyDown(event->key.keysym.sym, is);
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
        case SDL_WINDOWEVENT_RESIZED:
            if (is->targetRect) free(is->targetRect);
            is->targetRect = NULL;
            break;
    }
}

PLAY_STATE playURI(char *filename) {

    LOGI("开始播放 - %s", filename);

    //创建VideoState
    VideoState *is = createVideoState(filename);

    //创建读包线程（音频线程和视频线程也由读包线程打开）
    if (startReadPktThread(is) == 0) {
        //消息循环
        SDL_Event event;
        for (; ;) {
            if (SDL_WaitEventTimeout(&event, 20)) {
                //处理消息
                onHandMessage(&event, is);
            }
            //如果播放状态是出错，播放完毕，主动退出，则跳出消息循环
            if (is->play_state != PLAYING && is->play_state != PAUSE)
                break;
        }
    }

    //保持状态
    PLAY_STATE hr = is->play_state;

    is->audioq.enable = 0;
    is->videoq.enable = 0;
    SDL_CondSignal(is->videoq.cond);
    SDL_CondSignal(is->audioq.cond);

    //等待其他线程退出
    int state;
    if (is->readPkt_tid) SDL_WaitThread(is->readPkt_tid, &state);
    if (is->video_tid) SDL_WaitThread(is->video_tid, &state);

    //销毁VideoState
    destroyVideoState(is);

    return hr;
}