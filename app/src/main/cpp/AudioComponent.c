//
// Created by xiang on 2017-4-8.
//

#include <SDL_audio.h>
#include <libswresample/swresample.h>
#include "AudioComponent.h"
#include "LOG.h"

AVFrame frame;

/**
 * 音频重采样
 */
int audioResample(VideoState *is, AVFrame *af) {
    int resampled_data_size = 0;

    //获取源帧的音频数据大小
    int decoded_data_size = av_samples_get_buffer_size(NULL,
                                                       af->channels,
                                                       af->nb_samples,
                                                       af->format,
                                                       1);
    //获取源帧的声道布局
    int64_t dec_channel_layout =
            (af->channel_layout &&
             af->channels == av_get_channel_layout_nb_channels(af->channel_layout)) ?
            af->channel_layout : av_get_default_channel_layout(av_frame_get_channels(af));

    int wanted_nb_samples = af->nb_samples;

    //如果源帧的数据格式、采样率、声道布局有任意一项与我们之前约定的不一样则需要重新设置格式转换器
    if (af->format != is->audio_src_fmt ||
        af->sample_rate != is->audio_src_freq ||
        dec_channel_layout != is->audio_src_channel_layout ||
        (wanted_nb_samples != af->nb_samples && !is->swr_ctx)) {
        if (is->swr_ctx)
            swr_free(&(is->swr_ctx));
        is->swr_ctx = swr_alloc_set_opts(NULL,
                                         is->audio_tgt_channel_layout, is->audio_tgt_fmt,
                                         is->audio_tgt_freq,
                                         dec_channel_layout, af->format, af->sample_rate,
                                         0, NULL);
        if (!is->swr_ctx || swr_init(is->swr_ctx) < 0) {
            LOGE("创建音频格式转换器失败: %d Hz %s %d channels 到 %d Hz %s %d channels.\n",
                 af->sample_rate, av_get_sample_fmt_name(af->format), av_frame_get_channels(af),
                 is->audio_tgt_freq, av_get_sample_fmt_name(is->audio_tgt_fmt),
                 is->audio_tgt_channels);
            return -1;
        }
        is->audio_src_channel_layout = dec_channel_layout;
        is->audio_src_channels = is->audio_st->codec->channels;
        is->audio_src_fmt = is->audio_st->codec->sample_fmt;
        is->audio_src_freq = is->audio_st->codec->sample_rate;
    }

    //转换音频格式
    if (is->swr_ctx) {
        const uint8_t **in = (const uint8_t **) af->extended_data;
        uint8_t *out[] = {is->audio_buf2};
        if (wanted_nb_samples != af->nb_samples) {
            if (swr_set_compensation(is->swr_ctx,
                                     (wanted_nb_samples - af->nb_samples) * is->audio_tgt_freq /
                                     af->sample_rate,
                                     wanted_nb_samples * is->audio_tgt_freq / af->sample_rate) <
                0) {
                LOGE("audioResample:swr_set_compensation() failed.\n");
                return -1;
            }
        }

        int len2 = swr_convert(is->swr_ctx, out, sizeof(is->audio_buf2) / is->audio_tgt_channels /
                                                 av_get_bytes_per_sample(is->audio_tgt_fmt), in,
                               af->nb_samples);
        if (len2 < 0) {
            LOGE("audioResample:swr_convert() 失败.\n");
            return -1;
        }
        if (len2 == sizeof(is->audio_buf2) / is->audio_tgt_channels /
                    av_get_bytes_per_sample(is->audio_tgt_fmt)) {
            LOGI("音频缓冲区可能太小了.\n");
            swr_init(is->swr_ctx);  //TODO: 为什么要重新初始化swr_ctx
        }
        is->audio_buf = is->audio_buf2;
        resampled_data_size = len2 * is->audio_tgt_channels *
                              av_get_bytes_per_sample(is->audio_tgt_fmt);
    }

    else {
        is->audio_buf = af->data[0];
        resampled_data_size = decoded_data_size;
    }

    int n = 2 * is->audio_st->codec->channels;
    is->audio_clock += (double) resampled_data_size
                       / (double) (n * is->audio_st->codec->sample_rate);

    return
            resampled_data_size;
}

/**
 * 解码音频帧
 */
int audio_decode_frame(VideoState *is, double *pts_ptr) {
    AVPacket *pkt = av_packet_alloc();
    uint8_t *audio_pkt_data = NULL;
    int audio_pkt_size = 0;
    int len1, data_size = 0;

    //循环取包解码，直到获取到一个完整的音频数据帧
    for (; ;) {
        //不处于播放阶段不解码
        if (is->play_state != PLAYING) return -1;
        while (audio_pkt_size > 0) {
            int got_frame = 0;
            len1 = avcodec_decode_audio4(is->audio_st->codec, &frame, &got_frame, pkt);
            if (len1 < 0) {
                LOGD("audio_decode_frame:avcodec_decode_audio4()失败.\n");
                //解码错误，跳过此帧
                audio_pkt_size = 0;
                break;
            }
            audio_pkt_data += len1;
            audio_pkt_size -= len1;
            data_size = 0;
            if (got_frame) { //如果获取到一个完整帧，就重采样
                data_size = audioResample(is, &frame);
            }
            if (data_size <= 0) {
                /* 如果没有数据，则继续解码 */
                continue;
            }
            return data_size;
        }
        if (pkt->data) {
            av_packet_free(pkt);
        }
        memset(pkt, 0, sizeof(*pkt));
        //从列队中读取一个音频包
        if (packet_queue_get(&is->audioq, pkt, 1) < 0)
            return -1;  //列队中没有包了
        audio_pkt_data = pkt->data;
        audio_pkt_size = pkt->size;

        //更新音频时钟
        if (pkt->pts != AV_NOPTS_VALUE) {
            is->audio_clock = av_q2d(is->audio_st->time_base) * pkt->pts;
        }
    }
}

void audio_callback(void *userdata, Uint8 *stream, int len) {
    VideoState *is = (VideoState *) userdata;
    int len1, audio_data_size;
    double pts = 0;

    //len是由SDL传入的SDL缓冲区的大小，如果这个缓冲未满，我们就一直往里填充数据
    while (len > 0) {
        // audio_buf_index 和 audio_buf_size 标示我们自己用来放置解码出来的数据的缓冲区
        // 这些数据待copy到SDL缓冲区, 当audio_buf_index >= audio_buf_size的时候意味着
        // 我们的缓冲为空，没有数据可供copy，这时候需要调用audio_decode_frame来解码出更多的桢数据
        if (is->audio_buf_index >= is->audio_buf_size) {
            audio_data_size = audio_decode_frame(is, &pts);
            if (audio_data_size < 0)  // 没能解码出音频数据，我们默认播放静音 */
            {
                /* 填充1024个0（字节0为静音） */
                is->audio_buf_size = 1024;
                is->audio_buf = is->audio_buf2;
                memset(is->audio_buf, 0, is->audio_buf_size);
            } else {
                is->audio_buf_size = audio_data_size;
            }
            is->audio_buf_index = 0;
        }
        // 查看stream可用空间，决定一次copy多少数据，剩下的下次继续copy
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len) {
            len1 = len;
        }
        memcpy(stream, (uint8_t *) is->audio_buf + is->audio_buf_index, len1);
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
}

int audio_stream_component_open(VideoState *is, int stream_index) {
    AVFormatContext *ic = is->ic;

    if (stream_index < 0 || stream_index >= ic->nb_streams) {
        LOGE("音频流索引(%d)异常，小于0或大于最大值%d: %s\n", stream_index, ic->nb_streams);
        return -1;
    }

    AVCodecContext *codecCtx = ic->streams[stream_index]->codec;

    int wanted_nb_channels = codecCtx->channels; //期望（和源声道数一样的）的声道数
    int wanted_channel_layout = 0;

    if (!wanted_channel_layout //如果没设置声道布局
        || wanted_nb_channels   //或者期望的声道数不等于默认声道数对应的声道布局
           != av_get_channel_layout_nb_channels(
            wanted_channel_layout)) {
        //设置声道布局为期望的声道数对应的布局
        wanted_channel_layout = av_get_default_channel_layout(
                wanted_nb_channels);
        //取消立体声？？
        wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }

    SDL_AudioSpec wanted_spec, spec;

    //设置期望的音频设置

    wanted_spec.channels = av_get_channel_layout_nb_channels(
            wanted_channel_layout);
    wanted_spec.freq = codecCtx->sample_rate;
    if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
        LOGE("错误的采样率(%d)或声道(%d)\n", wanted_spec.freq, wanted_spec.channels);
        return -1;
    }
    wanted_spec.format = AUDIO_S16SYS; // 具体含义请查看“SDL宏定义”部分
    wanted_spec.silence = 0;            // 0指示静音
    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;  // 自定义SDL缓冲区大小
    wanted_spec.callback = audio_callback;        // 音频数据不足时的回调函数
    wanted_spec.userdata = is;                    // 传给上面回调函数的外带数据

    /*  打开音频设备，循环尝试打开不同的声道数
     * (next_nb_channels）直到成功打开，或者全部失败 */
    const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6}; //SDL支持的声道数为 1, 2, 4, 6
    while (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
        LOGD("尝试%d声道失败: %s\n", wanted_spec.channels, SDL_GetError());
        //设置下一个尝试的声道数
        wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
        if (!wanted_spec.channels) {
            LOGE("打开音频设备失败，没有匹配的声道.\n");
            return -1;
        }
        //设置下一个尝试的声道布局
        wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
    }

    /* 检查实际使用的配置（保存在spec,由SDL_OpenAudio()填充） */
    if (spec.format != AUDIO_S16SYS) {
        LOGE("SDL建议的音频格式（%d）暂不支持.\n", spec.format);
        return -1;
    }
    if (spec.channels != wanted_spec.channels) {
        wanted_channel_layout = av_get_default_channel_layout(spec.channels);
        if (!wanted_channel_layout) {
            LOGE("SDL建议的声道数（%d）暂不支持.\n", spec.channels);
            return -1;
        }
    }

    //保持到VideoState中
    is->audio_hw_buf_size = spec.size;
    is->audio_src_fmt = is->audio_tgt_fmt = AV_SAMPLE_FMT_S16;
    is->audio_src_freq = is->audio_tgt_freq = spec.freq;
    is->audio_src_channel_layout = is->audio_tgt_channel_layout = wanted_channel_layout;
    is->audio_src_channels = is->audio_tgt_channels = spec.channels;

    //查找并打开对应的音频解码器
    AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
    if (!codec || (avcodec_open2(codecCtx, codec, NULL) < 0)) {
        LOGE("音频格式不支持\n");
        return -1;
    }
    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT; //TODO: 这个有什么用
    switch (codecCtx->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            is->audioStream = stream_index;
            is->audio_st = ic->streams[stream_index];
            is->audio_buf_size = 0;
            is->audio_buf_index = 0;
            packet_queue_init(&is->audioq);
            //开始播放
            SDL_PauseAudio(0);
            break;
        default:
            break;
    }
    return 0;
}