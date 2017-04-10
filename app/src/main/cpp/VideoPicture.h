//
// Created by xiang on 2017-4-8.
//

#ifndef PLAYER_VIDEOPICTURE_H
#define PLAYER_VIDEOPICTURE_H


#include <libavutil/frame.h>
#include <SDL_render.h>

/**
 * 用来保存解码出来的图像
 */
typedef struct VideoPicture {
    SDL_Window *screen;
    SDL_Renderer *renderer;
    SDL_Texture *bmp;

    AVFrame *rawdata; //格式转换时中间数据帧

    int width, height; //源图像的宽度和高度

    int allocated;
    double pts;
} VideoPicture;

#endif //PLAYER_VIDEOPICTURE_H
