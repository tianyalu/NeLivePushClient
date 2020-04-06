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
