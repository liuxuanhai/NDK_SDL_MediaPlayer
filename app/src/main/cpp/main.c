//
// 视频播放的入口
// Created by xiang on 2017-4-8.
//

#include <SDL.h>
#include <libavformat/avformat.h>
#include "LOG.h"
#include "Player.h"

/**
 * Main 函数
 */
int main(int argc, char *argv[]) {
    //判断参数
    if (argc < 2) {
        //如果程序没有附加参数
        LOGI("将要播放的文件添加至命令行即可播放.\n");
        exit(0);
    }

    //注册所有ffmpeg的解码器
    av_register_all();

    //初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        LOGE("初始化SDL失败: %s\n", SDL_GetError());
        exit(1);    //SDL初始化失败，直接退出，不需要SDL_Quit();
    }

    //将命令行附加的文件依次提取出来播放
    for (int i = 1; i < argc; ++i) {
        char *filename = argv[i];
        PLAY_STATE result = playURI(filename);
        if (result == QUIT) { //如果是退出则直接跳出循环，后面的文件也不播放了
            break;
        }
    }
    SDL_Quit();
    return 0;
}