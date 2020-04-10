#include <jni.h>
#include <string>
#include <rtmp.h>
#include <x264.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_sty_ne_livepushclient_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    char version[100];
    sprintf(version, "rtmp version : %d", RTMP_LibVersion());
    //测试x264
    x264_picture_t *pic = new x264_picture_t;
    x264_picture_init(pic);
    return env->NewStringUTF(version);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_initNative(JNIEnv *env, jobject thiz) {
    // TODO: implement initNative()
}

extern "C" JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_startLiveNative(JNIEnv *env, jobject thiz, jstring path) {
    // TODO: implement startLiveNative()
}

extern "C" JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_stopLiveNative(JNIEnv *env, jobject thiz) {
    // TODO: implement stopLiveNative()
}

extern "C" JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_initVideoEncoderNative(JNIEnv *env, jobject thiz,
                                                                 jint width, jint height,
                                                                 jint bitrate, jint fps) {
    // TODO: implement initVideoEncoderNative()
}

extern "C" JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_pushVideoNative(JNIEnv *env, jobject thiz,
                                                          jbyteArray data) {
    // TODO: implement pushVideoNative()
}

extern "C" JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_releaseNative(JNIEnv *env, jobject thiz) {
    // TODO: implement releaseNative()
}