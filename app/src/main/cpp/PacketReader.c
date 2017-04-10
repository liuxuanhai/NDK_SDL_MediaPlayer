//
// Created by xiang on 2017-4-8.
//

#include <SDL_messagebox.h>
#include <SDL_timer.h>
#include <SDL_events.h>
#include "PacketReader.h"
#include "LOG.h"
#include "AudioComponent.h"
#include "VideoComponent.h"

/**
 * 判断是否要继续等待输入流
 */
int decode_interrupt_cb(void *opaque) {
    VideoState *is = (VideoState *) (opaque);
    if (is) {
        if (is->play_state == FINISH || is->play_state == QUIT)
            //通知ffmpeg放弃阻塞
            return 1;
    }
    //通知ffmpeg继续阻塞等待输入流
    return 0;
}

/*
 * 视频信息分析和读包线程
 */
int readPkt_thread(void *arg) {
    int err_code;
    VideoState *is = (VideoState *) (arg);
    is->play_state = PLAYING;

    //设置输入流等待的回调，不然会一直等待输入流
    AVIOInterruptCB interupt_cb;
    interupt_cb.callback = decode_interrupt_cb;
    interupt_cb.opaque = is;

    //根据文件路径初始化输入流上下文
    if ((err_code = avio_open2(&is->io_ctx, is->filename, 0, &interupt_cb, NULL)) < 0) {
        LOGE("无法识别播放源code(%d) - %s\n", err_code, is->filename);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "播放失败",
                                 "初始化播放源失败，无法识别播放源",
                                 NULL);
        goto error;
    }

    //打开文件并读取文件头部信息
    if ((err_code = avformat_open_input(&(is->ic), is->filename, NULL, NULL))) {
        LOGE("打开输入流失败code(%d) - %s\n", err_code, is->filename);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "播放失败",
                                 "打开文件失败，文件可能已不存在或不可访问或不是有效的视频文件",
                                 NULL);
        goto error;
    }

    //查找流信息
    if ((err_code = avformat_find_stream_info(is->ic, NULL)) < 0) {
        LOGE("查找流信息失败code(%d) - %s\n", err_code, is->filename);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "播放失败",
                                 "查找流信息失败",
                                 NULL);
        goto error;
    }

    //打印格式信息到stdout
    av_dump_format(is->ic, 0, is->filename, 0);

    //查找视频和音频流
    int video_index = -1, audio_index = -1, video_ok = -1, audio_ok = -1;
    for (int i = 0; i < is->ic->nb_streams; i++) {
        if (is->ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO
            && video_index < 0) {
            video_index = i;
        }
        if (is->ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO
            && audio_index < 0) {
            audio_index = i;
        }
    }
    //打开音频
    if (audio_index >= 0) {
        audio_ok = audio_stream_component_open(is, audio_index);
    } else {
        LOGE("没有找到音频流 - %s\n", is->filename);
    }
    //打开视频
    if (video_index >= 0) {
        video_ok = video_stream_component_open(is, video_index);
    } else {
        LOGE("没有找到视频流 - %s\n", is->filename);
    }
    //如果没有音频，也没有视频，则直接取消播放
    if (audio_ok != 0 && video_ok != 0) {
        LOGE("该视频没有任何音频或视频流.\n");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "播放失败",
                                 "无法播放，由于没有找到或成功打开任何音频流和视频流",
                                 NULL);
        goto error;
    }

    AVPacket pkt1, *packet = &pkt1;
    //开始循环读包
    LOGD("读包线程已开始.\n");
    schedule_refresh(is, 40);
    for (; ;) {
        if (is->play_state == QUIT) {
            break;
        }
        //如果列队数据已满，则等待
        if (is->audioq.size > MAX_AUDIO_SIZE || is->videoq.size > MAX_VIDEO_SIZE) {
            SDL_Delay(10);
            continue;
        }
        //读取下一个数据帧
        if (av_read_frame(is->ic, packet) < 0) {
            if (is->ic->pb->error == 0) { //输入流数据未到，则等待
                SDL_Delay(100);
                continue;
            } else {    //读取出现异常或输入流已经结尾
                LOGD("读取包出现异常或输入流已经结尾.\n");
                break;
            }
        }

        if (packet->stream_index == is->videoStream) {
            packet_queue_put(&is->videoq, packet);
        } else if (packet->stream_index == is->audioStream) {
            packet_queue_put(&is->audioq, packet);
        } else {
            av_packet_free(packet);
        }
    }
    LOGD("读包线程已结束.\n");
    return 0;

    error:
    is->play_state = ERROR;
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
    return -1;
}

int startReadPktThread(VideoState *is) {
    is->readPkt_tid = SDL_CreateThread(readPkt_thread, "readPkt_thread", is);//解码线程
    if (!is->readPkt_tid) {
        LOGE("创建读包线程失败.\n");
        is->play_state = ERROR;
        return -1;
    }
    return 0;
}