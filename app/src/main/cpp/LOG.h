//
// 日志工具
// Created by xiang on 2017-4-8.
// 这里我们只定义了3种级别的日志
//

#ifndef PLAYER_LOG_H
#define PLAYER_LOG_H

#ifdef __ANDROID__
//如果是android就用anroid 的 log
#include <android/log.h>
//这个是自定义的LOG的标识
#define TAG "PlayerJni"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#else
//如果是其他平台就用 fprintf
#include <stdio.h>
#define LOGD(...) fprintf(stdout, __VA_ARGS__);
#define LOGI(...) fprintf(stdout, __VA_ARGS__);
#define LOGE(...) fprintf(stderr, __VA_ARGS__);
#endif

#endif //PLAYER_LOG_H
