//
// Created by xiang on 2017-4-8.
//

#ifndef MEDIA_SDL_ANDROID_H
#define MEDIA_SDL_ANDROID_H

/*
    SDL_android_main.c, placed in the public domain by Sam Lantinga  3/13/14
*/
#include "sdlmain/SDL_internal.h"

#ifdef __ANDROID__

#include "SDL_main.h"

/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/
#include <jni.h>

JNIEXPORT int JNICALL Java_xwc_media_android_PlayActivity_nativeInit(JNIEnv *env, jclass cls,
                                                                     jobject array);

JNIEXPORT int JNICALL Java_xwc_media_android_PlayActivity_onNativeResize(JNIEnv *env, jclass cls,
                                                                         jint a, jint b, jint c,
                                                                         jfloat d);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_onNativeSurfaceChanged(JNIEnv *env,
                                                                                  jclass cls);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_onNativeAccel(JNIEnv *env, jclass cls,
                                                                         jfloat a, jfloat b,
                                                                         jfloat c);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_onNativeTouch(JNIEnv *env, jclass cls,
                                                                         jint, jint, jint, jfloat,
                                                                         jfloat, jfloat);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_onNativeKeyDown(JNIEnv *env, jclass cls,
                                                                           jint);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_onNativeKeyUp(JNIEnv *env, jclass cls,
                                                                         jint);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_nativePause(JNIEnv *env, jclass cls);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_nativeResume(JNIEnv *env, jclass cls);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_onNativeSurfaceDestroyed(JNIEnv *env,
                                                                                    jclass cls);


JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_nativeQuit(JNIEnv *env,
                                                                      jclass cls);

#endif /* __ANDROID__ */

#endif //MEDIA_SDL_ANDROID_H
