//
// Created by xiang on 2017-4-8.
//

#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include <SDL_events.h>
#include <libavutil/imgutils.h>
#include <SDL_timer.h>
#include "VideoComponent.h"
#include "LOG.h"

void alloc_picture(VideoState *is) {
    VideoPicture *vp;

    vp = &is->pictq[is->pictq_windex];

    if (is->bmp) {
        SDL_DestroyTexture(is->bmp);
    }

    if (vp->rawdata) {
        av_free(vp->rawdata);
    }

    vp->width = is->video_st->codec->width;
    vp->height = is->video_st->codec->height;

    if (is->screen == NULL) {
        int flags = SDL_WINDOW_OPENGL;
#ifdef __ANDROID__
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif
        is->screen = SDL_CreateWindow("MediaPlayer", //窗口的标题
                                      SDL_WINDOWPOS_UNDEFINED,//窗口的位置信息
                                      SDL_WINDOWPOS_UNDEFINED,
                                      vp->width,
                                      vp->height,//窗口的大小（长、宽
                                      flags);//窗口的 状态
    }

    if (is->renderer == NULL) {
        is->renderer = SDL_CreateRenderer(is->screen, -1, 0);
    }
    is->bmp = SDL_CreateTexture(is->renderer,
                                SDL_PIXELFORMAT_YV12,
                                SDL_TEXTUREACCESS_STREAMING,
                                is->video_st->codec->width,
                                is->video_st->codec->height);

    AVFrame *pFrameYUV = av_frame_alloc();
    if (pFrameYUV == NULL)
        return;

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, vp->width, vp->height, 1);

    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize,
                         buffer, AV_PIX_FMT_YUV420P, vp->width, vp->height, 1);

    vp->rawdata = pFrameYUV;

    SDL_LockMutex(is->pictq_mutex);
    vp->allocated = 1;
    SDL_CondSignal(is->pictq_cond);
    SDL_UnlockMutex(is->pictq_mutex);
}

int queue_picture(VideoState *is, AVFrame *pFrame,
                  double pts) {
    VideoPicture *vp;
    AVPicture pict;

    SDL_LockMutex(is->pictq_mutex);
    while (is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE && !is->play_state != PLAYING) {
        SDL_CondWaitTimeout(is->pictq_cond, is->pictq_mutex, 10);
    }
    SDL_UnlockMutex(is->pictq_mutex);

    if (is->play_state != PLAYING)
        return -1;

    vp = &is->pictq[is->pictq_windex];

    //如果图片跟显示区域大小不一致，需要通知主线程重新创建显示对象
    if (!is->bmp || vp->width != is->video_st->codec->width
        || vp->height != is->video_st->codec->height) {
        SDL_Event event;

        vp->allocated = 0;
        //通知主线程更新界面设置
        event.type = FF_ALLOC_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);

        //等待主线程把界面准备好
        SDL_LockMutex(is->pictq_mutex);
        while (!vp->allocated && is->play_state != PLAYING) {
            SDL_CondWait(is->pictq_cond, is->pictq_mutex);
        }
    }
    SDL_UnlockMutex(is->pictq_mutex);
    if (is->play_state != PLAYING) {
        return -1;
    }

    if (vp->rawdata) {
        // 把图像数据转换为YUV格式
        sws_scale(is->sws_ctx, (uint8_t const *const *) pFrame->data,
                  pFrame->linesize, 0, is->video_st->codec->height,
                  vp->rawdata->data, vp->rawdata->linesize);
        vp->pts = pts;
        //把图片放入列队
        if (++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE) {
            is->pictq_windex = 0;
        }
        SDL_LockMutex(is->pictq_mutex);
        is->pictq_size++;
        SDL_UnlockMutex(is->pictq_mutex);
    }
    return 0;
}

double synchronize_video(VideoState *is, AVFrame *src_frame, double pts) {
    double frame_delay;

    if (pts != 0) {
        is->video_clock = pts;
    } else {
        pts = is->video_clock;
    }
    frame_delay = av_q2d(is->video_st->codec->time_base);
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    is->video_clock += frame_delay;
    return pts;
}

int video_thread(void *arg) {
    VideoState *is = (VideoState *) arg;
    AVPacket pkt1, *packet = &pkt1;
    AVFrame *pFrame = av_frame_alloc();
    double pts = 0;
    int frameFinished;

    for (; ;) {
        //从列队中读包
        if (packet_queue_get(&is->videoq, packet, 1) < 0) {
            //已经没有任何数据了，该退出了
            break;
        }
        //解码这个包
        avcodec_decode_video2(is->video_st->codec, pFrame, &frameFinished,
                              packet);

        //获取帧的pts
        if (packet->dts == AV_NOPTS_VALUE && pFrame->opaque
            && *(uint64_t *) pFrame->opaque != AV_NOPTS_VALUE) {
            pts = *(uint64_t *) pFrame->opaque;
        } else if (packet->dts != AV_NOPTS_VALUE) {
            pts = packet->dts;
        } else {
            pts = 0;
        }
        pts *= av_q2d(is->video_st->time_base);

        //如果已经满一帧了
        if (frameFinished) {
            pts = synchronize_video(is, pFrame, pts);
            if (queue_picture(is, pFrame, pts) < 0) {
                break;
            }
        }

        av_packet_unref(packet);
    }

    if (pFrame) av_free(pFrame);

    return 0;
}

int video_stream_component_open(VideoState *is, int stream_index) {
    AVFormatContext *pFormatCtx = is->ic;

    if (stream_index < 0 || stream_index >= pFormatCtx->nb_streams) {
        LOGE("视频流索引(%d)异常，小于0或大于最大值%d: %s\n", stream_index, pFormatCtx->nb_streams);
        return -1;
    }

    AVCodecContext *codecCtx = pFormatCtx->streams[stream_index]->codec;

    //查找并打开对应的视频解码器
    AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
    if (!codec || (avcodec_open2(codecCtx, codec, NULL) < 0)) {
        LOGE("视频格式不支持\n");
        return -1;
    }

    switch (codecCtx->codec_type) {
        case AVMEDIA_TYPE_VIDEO:
            is->videoStream = stream_index;
            is->video_st = pFormatCtx->streams[stream_index];
            is->sws_ctx = sws_getContext(is->video_st->codec->width,
                                         is->video_st->codec->height, is->video_st->codec->pix_fmt,
                                         is->video_st->codec->width, is->video_st->codec->height,
                                         AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
            is->frame_timer = (double) av_gettime() / 1000000.0;//加的
            is->frame_last_delay = 40e-3;
            packet_queue_init(&is->videoq);
            is->video_tid = SDL_CreateThread(video_thread, "video_thread", is); //创建解码线程
            break;
        default:
            break;
    }
    return 0;
}

static Uint32 sdl_refresh_timer_cb(Uint32 interval, void *opaque) {
    VideoState *is = (VideoState *) opaque;
    SDL_Event event;
    /*其实，或许可以自定义事件*/
    if (is->play_state == PLAYING) {
        event.type = FF_REFRESH_EVENT;
        event.user.data1 = opaque;
        SDL_PushEvent(&event);
    } else if (is->play_state == PAUSE) {
        //等待暂停
        while (is->play_state == PAUSE) {
            SDL_Delay(10);
        }
    }
    return 0;
}

//设置一个延迟时间,系统指定的毫秒数后推FF_REFRESH_EVENT。当我们看到它在事件队列时，将依次调用视频刷新功能。
void schedule_refresh(VideoState *is,
                      int delay) {
    SDL_AddTimer(delay, sdl_refresh_timer_cb, is);
}


void video_refresh_timer(VideoState *is) {
    VideoPicture *vp;
    double actual_delay, delay, sync_threshold, ref_clock, diff;
    if (is->play_state != PLAYING) return;
    if (is->video_st) {
        if (is->pictq_size == 0) {
            schedule_refresh(is, 1);
        }
        else {
            vp = &is->pictq[is->pictq_rindex];//加的

            delay = vp->pts - is->frame_last_pts;
            if (delay <= 0 || delay >= 1.0) {
                delay = is->frame_last_delay;
            }
            is->frame_last_delay = delay;
            is->frame_last_pts = vp->pts;

            if (is->audioStream > 0) {
                ref_clock = get_audio_clock(is);
                diff = vp->pts - ref_clock;
            } else {
                diff = 0;
            }

            /* Skip or repeat the frame. Take delay into account
             FFPlay still doesn't "know if this is the best guess." */
            sync_threshold = (delay > AV_SYNC_THRESHOLD) ? delay : AV_SYNC_THRESHOLD;
            if (fabs(diff) < AV_NOSYNC_THRESHOLD) {
                if (diff <= -sync_threshold) {
                    delay = 0.0;
                } else if (diff >= sync_threshold) {
                    delay = 2 * delay;
                }
            }
            is->frame_timer += delay;
            /* computer the REAL delay */
            actual_delay = is->frame_timer - (av_gettime() / 1000000.0);
            if (actual_delay < 0.010) {
                /* Really it should skip the picture instead */
                actual_delay = 0.010;
            }
            schedule_refresh(is, (int) (actual_delay * 1000 + 0.5));

            /* 显示图片 */
            video_display(is);

            /* update queue for next picture! */
            if (++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE) {
                is->pictq_rindex = 0;
            }
            SDL_LockMutex(is->pictq_mutex);
            is->pictq_size--;
            SDL_CondSignal(is->pictq_cond);
            SDL_UnlockMutex(is->pictq_mutex);
        }
    } else {
        schedule_refresh(is, 100);
    }
}

void calculateTargetRect(VideoState *is) {
    if (is->targetRect) free(is->targetRect);
    is->targetRect = malloc(sizeof(SDL_Rect));
    double aspect_ratio;
    int winW = 0, winH = 0;
    if (is->screen) SDL_GetWindowSize(is->screen, &winW, &winH);
    switch (is->show_mode) {
        case FULL_SCREEN:
            is->targetRect->x = 0;
            is->targetRect->y = 0;
            is->targetRect->w = winW;
            is->targetRect->h = winH;
            break;
        case SRC_SIZE: //居中
            is->targetRect->w = is->video_st->codec->width;
            is->targetRect->h = is->video_st->codec->height;
            is->targetRect->x = (winW - is->targetRect->w) / 2;
            is->targetRect->y = (winH - is->targetRect->h) / 2;
            break;
        case MAX_SIZE_RATIO:
            //计算比例，然后根据比例进行最大缩放，若屏幕和源的尺寸比例不一致将有黑边

            //计算源画面宽高比例
            if (is->video_st->codec->sample_aspect_ratio.num == 0) {
                aspect_ratio = 0;
            } else {
                aspect_ratio = av_q2d(is->video_st->codec->sample_aspect_ratio)
                               * is->video_st->codec->width / is->video_st->codec->height;
            }
            if (aspect_ratio <= 0.0) {
                aspect_ratio = (float) is->video_st->codec->width
                               / (float) is->video_st->codec->height;
            }

            if (aspect_ratio > ((double) winW / winH)) {    //width 最大化
                is->targetRect->x = 0;
                is->targetRect->w = winW;
                is->targetRect->h =
                        is->video_st->codec->height *
                        ((double) winW / is->video_st->codec->width);
                is->targetRect->y = (winH - is->targetRect->h) / 2; //竖直居中
            } else { //height 最大化
                is->targetRect->y = 0;
                is->targetRect->h = winH;
                is->targetRect->w = is->video_st->codec->width *
                                    ((double) winH / is->video_st->codec->height);
                is->targetRect->x = (winW - is->targetRect->w) / 2; //水平居中
            }
            break;
    }
}

void video_display(VideoState *is) {
    SDL_Rect rect, desc;
    VideoPicture *vp;
    vp = &is->pictq[is->pictq_rindex];
    if (is->bmp) {
        rect.x = 0;
        rect.y = 0;
        rect.w = vp->width;
        rect.h = vp->height;

        SDL_UpdateYUVTexture(is->bmp, &rect,
                             vp->rawdata->data[0], vp->rawdata->linesize[0],
                             vp->rawdata->data[1], vp->rawdata->linesize[1],
                             vp->rawdata->data[2], vp->rawdata->linesize[2]);

        SDL_RenderClear(is->renderer);
        if (is->targetRect == NULL) {
            calculateTargetRect(is);
        }
        SDL_RenderCopy(is->renderer, is->bmp, &rect, is->targetRect);
        SDL_RenderPresent(is->renderer);
    }
}