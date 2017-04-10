//
// Created by xiang on 2017-4-8.
//

#ifndef PLAYER_VIDEOSTATE_H_H
#define PLAYER_VIDEOSTATE_H_H

#include <libavformat/avformat.h>
#include <SDL_render.h>
#include <SDL_mutex.h>
#include <SDL_thread.h>
#include "PacketQueue.h"
#include "VideoPicture.h"
#include "Player.h"

//视频队列大小
#define VIDEO_PICTURE_QUEUE_SIZE 1

//定义最大的音频缓存区大小
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

/**
 * 定义视频播放结构
 */
typedef struct VideoState {
    //视频路径
    char filename[1024];
    //文件IO上下文
    AVIOContext *io_ctx;
    //视频文件格式相关信息上下文
    AVFormatContext *ic;

    /** 音频相关 **/
    double audio_clock;
    //音频流索引
    int audioStream;
    //音频流
    AVStream *audio_st;
    //音频缓存区大小
    Uint32 audio_hw_buf_size;
    //音频缓存区
    uint8_t *audio_buf;
    DECLARE_ALIGNED(16, uint8_t, audio_buf2)[AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];
    //音频缓存区现存音频数据的大小
    int audio_buf_size;
    //音频缓存区现在的读取起点（并不是每次都从头开始读取）
    int audio_buf_index;
    //音频包列队
    PacketQueue audioq;
    //源音频格式
    enum AVSampleFormat audio_src_fmt;
    //目标音频格式
    enum AVSampleFormat audio_tgt_fmt;
    //源音频声道
    int audio_src_channels;
    //目标音频声道
    int audio_tgt_channels;   //通道数
    //源音频声道布局
    int64_t audio_src_channel_layout;
    //目标音频声道布局
    int64_t audio_tgt_channel_layout;
    //源音频采样率
    int audio_src_freq;
    //目标音频采样率
    int audio_tgt_freq;
    //音频格式转换器
    struct SwrContext *swr_ctx;

    /** 视频相关 **/
    double video_clock;
    //视频流索引
    int videoStream;
    //视频流
    AVStream *video_st;
    //视频包队列
    PacketQueue videoq;
    //视频画面转换器（包括图像格式和画面缩放）
    struct SwsContext *sws_ctx;
    //TODO: 添加注释
    double frame_timer;
    //TODO: 添加注释
    double frame_last_pts;
    //TODO: 添加注释
    double frame_last_delay;
    //图片列队（缓存的视频图片）
    VideoPicture pictq[VIDEO_PICTURE_QUEUE_SIZE];
    //图片队列大小、图片队列尾部，图片队列头部
    int pictq_size, pictq_rindex, pictq_windex;
    //等待图片队列有空闲的锁
    SDL_mutex *pictq_mutex;
    //通知图片队列有空闲的信号
    SDL_cond *pictq_cond;

    //视频信息分析和读包线程
    SDL_Thread *readPkt_tid;
    //视频播放线程
    SDL_Thread *video_tid;

    //播放状态
    PLAY_STATE play_state;
} VideoState;

/** Functions **/

/**
 * 创建一个VideoState
 */
VideoState *createVideoState(char *filename);

/**
 * 销毁一个VideoState
 */
void destroyVideoState(VideoState *is);

/**
 * 获取当前音频播放时间
 */
double get_audio_clock(VideoState *is);

#endif //PLAYER_VIDEOSTATE_H_H
