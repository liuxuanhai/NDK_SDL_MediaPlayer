//
// Created by xiang on 2017-4-8.
//

#include <libavutil/avstring.h>
#include "VideoState.h"

VideoState *createVideoState(char *filename) {
    VideoState *is = av_malloc(sizeof(VideoState));
    av_strlcpy(is->filename, filename, sizeof(is->filename));
    is->pictq_mutex = SDL_CreateMutex();
    is->pictq_cond = SDL_CreateCond();
    return is;
}

void destroyVideoState(VideoState *is) {
    if (is == NULL) return;

    if (is->pictq_mutex) {
        SDL_DestroyMutex(is->pictq_mutex);
        is->pictq_mutex = NULL;
    }

    if (is->pictq_cond) {
        SDL_DestroyCond(is->pictq_cond);
        is->pictq_cond = NULL;
    }

    av_free(is);
}

double get_audio_clock(VideoState *is) {
    double pts;
    int hw_buf_size, bytes_per_sec, n;

    pts = is->audio_clock; /* maintained in the audio thread */
    hw_buf_size = is->audio_buf_size - is->audio_buf_index;
    //每个样本占2bytes。16bit
    bytes_per_sec = 0;
    n = is->audio_st->codec->channels * 2;
    if (is->audio_st) {
        bytes_per_sec = is->audio_st->codec->sample_rate * n;
    }
    if (bytes_per_sec) {
        pts -= (double) hw_buf_size / bytes_per_sec;
    }
    return pts;
}