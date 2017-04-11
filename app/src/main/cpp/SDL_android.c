/*
    SDL_android_main.c, placed in the public domain by Sam Lantinga  3/13/14
*/
#include <SDL_events.h>
#include "SDL_android.h"

#ifdef __ANDROID__

/* Called before SDL_main() to initialize JNI bindings in SDL library */
extern void SDL_Android_Init(JNIEnv *env, jclass cls);

/* Start up the SDL app */
JNIEXPORT int JNICALL Java_xwc_media_android_PlayActivity_nativeInit(JNIEnv *env, jclass cls,
                                                                     jobject array) {
    int i;
    int argc;
    int status;
    int len;
    char **argv;
    /* This interface could expand with ABI negotiation, callbacks, etc. */
    SDL_Android_Init(env, cls);
    SDL_SetMainReady();
    /* Prepare the arguments. */
    len = (*env)->GetArrayLength(env, array);
    argv = SDL_stack_alloc(char*, 1 + len + 1);
    argc = 0;
    /* Use the name "app_process" so PHYSFS_platformCalcBaseDir() works.
       https://bitbucket.org/MartinFelis/love-android-sdl2/issue/23/release-build-crash-on-start
     */
    argv[argc++] = SDL_strdup("app_process");
    for (i = 0; i < len; ++i) {
        const char *utf;
        char *arg = NULL;
        jstring string = (*env)->GetObjectArrayElement(env, array, i);
        if (string) {
            utf = (*env)->GetStringUTFChars(env, string, 0);
            if (utf) {
                arg = SDL_strdup(utf);
                (*env)->ReleaseStringUTFChars(env, string, utf);
            }
            (*env)->DeleteLocalRef(env, string);
        }
        if (!arg) {
            arg = SDL_strdup("");
        }
        argv[argc++] = arg;
    }
    argv[argc] = NULL;
    /* Run the application. */
    status = SDL_main(argc, argv);
    /* Release the arguments. */
    for (i = 0; i < argc; ++i) {
        SDL_free(argv[i]);
    }
    SDL_stack_free(argv);
    /* Do not issue an exit or the whole application will terminate instead of just the SDL thread */
    /* exit(status); */
    return status;
}

extern int Java_org_libsdl_app_SDLActivity_onNativeResize(JNIEnv *, jclass, jint, jint, jint,
                                                          jfloat);

JNIEXPORT int JNICALL Java_xwc_media_android_PlayActivity_onNativeResize(JNIEnv *env, jclass cls,
                                                                         jint a, jint b, jint c,
                                                                         jfloat d) {
    SDL_Event event;
    event.type = SDL_WINDOWEVENT_RESIZED;
    SDL_PushEvent(&event);
    return Java_org_libsdl_app_SDLActivity_onNativeResize(env, cls, a, b, c, d);
}

extern void Java_org_libsdl_app_SDLActivity_onNativeSurfaceChanged(JNIEnv *, jclass);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_onNativeSurfaceChanged(JNIEnv *env,
                                                                                  jclass cls) {
    Java_org_libsdl_app_SDLActivity_onNativeSurfaceChanged(env, cls);
}

extern void Java_org_libsdl_app_SDLActivity_onNativeAccel(JNIEnv *, jclass, jfloat, jfloat, jfloat);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_onNativeAccel(JNIEnv *env, jclass cls,
                                                                         jfloat a, jfloat b,
                                                                         jfloat c) {
    Java_org_libsdl_app_SDLActivity_onNativeAccel(env, cls, a, b, c);
}

extern void Java_org_libsdl_app_SDLActivity_onNativeTouch(JNIEnv *env, jclass cls,
                                                          jint, jint, jint, jfloat,
                                                          jfloat, jfloat);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_onNativeTouch(JNIEnv *env, jclass cls,
                                                                         jint a, jint b, jint c,
                                                                         jfloat d,
                                                                         jfloat e, jfloat f) {
    Java_org_libsdl_app_SDLActivity_onNativeTouch(env, cls, a, b, c, d, e, f);
}

extern void Java_org_libsdl_app_SDLActivity_onNativeKeyDown(JNIEnv *env, jclass cls,
                                                            jint);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_onNativeKeyDown(JNIEnv *env, jclass cls,
                                                                           jint key) {
    Java_org_libsdl_app_SDLActivity_onNativeKeyDown(env, cls, key);
}

extern void Java_org_libsdl_app_SDLActivity_onNativeKeyUp(JNIEnv *env, jclass cls,
                                                          jint);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_onNativeKeyUp(JNIEnv *env, jclass cls,
                                                                         jint key) {
    Java_org_libsdl_app_SDLActivity_onNativeKeyUp(env, cls, key);
}

extern void Java_org_libsdl_app_SDLActivity_nativePause(JNIEnv *env, jclass cls);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_nativePause(JNIEnv *env, jclass cls) {
    Java_org_libsdl_app_SDLActivity_nativePause(env, cls);
    //TODO: pause
}

extern void Java_org_libsdl_app_SDLActivity_nativePause(JNIEnv *env, jclass cls);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_nativeResume(JNIEnv *env, jclass cls) {
    Java_xwc_media_android_PlayActivity_nativeResume(env, cls);
    //TODO: resume.
}

extern void Java_org_libsdl_app_SDLActivity_onNativeSurfaceDestroyed(JNIEnv *env, jclass cls);

JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_onNativeSurfaceDestroyed(JNIEnv *env,
                                                                                    jclass cls) {
    Java_org_libsdl_app_SDLActivity_onNativeSurfaceDestroyed(env, cls);
}

extern void Java_org_libsdl_app_SDLActivity_nativeQuit(JNIEnv *env, jclass cls);


JNIEXPORT void JNICALL Java_xwc_media_android_PlayActivity_nativeQuit(JNIEnv *env,
                                                                      jclass cls) {
    Java_org_libsdl_app_SDLActivity_nativeQuit(env, cls);
}

#endif /* __ANDROID__ */

/* vi: set ts=4 sw=4 expandtab: */
