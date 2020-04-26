#include <jni.h>
#include <string>
#include <rtmp.h>
#include <x264.h>
#include "VideoChannel.h"
#include "safe_queue.h"
#include "macro.h"
#include "AudioChannel.h"

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

VideoChannel *video_channel = 0;
AudioChannel *audio_channel = 0;
SafeQueue<RTMPPacket *> packets;
bool isStart;
pthread_t pid_start;
uint32_t start_time;

void ReleaseRTMPPacket(RTMPPacket **packet) {
    if(packet) {
        RTMPPacket_Free(*packet);
        delete *packet;
        *packet = 0;
    }
}

void callback(RTMPPacket *packet) {
    if(packet) {
        if(packet->m_nTimeStamp == -1) { //表示需要时间戳
            packet->m_nTimeStamp = RTMP_GetTime() - start_time;
        }
        packets.push(packet);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_initNative(JNIEnv *env, jobject thiz) {
    video_channel = new VideoChannel;
    video_channel->setVideoCallback(callback);
    packets.setReleaseCallback(ReleaseRTMPPacket);
}

void *task_start(void *args) {
    char *url = static_cast<char *>(args);
    RTMP *rtmp = 0;
    int ret;
    do {
        //1. rtmp初始化
        rtmp = RTMP_Alloc();
        if(!rtmp) {
            LOGE2("rtmp 初始失败");
            break;
        }
        RTMP_Init(rtmp);

        //2.设置流媒体地址
        ret = RTMP_SetupURL(rtmp, url);
        if(!ret) {
            LOGE2("设置流媒体地址失败");
            break;
        }

        //3.开启输出模式
        RTMP_EnableWrite(rtmp);

        //4.建立连接
        ret = RTMP_Connect(rtmp, 0);
        if(!ret) {
            LOGE2("建立连接失败");
            break;
        }

        //5.建立流连接
        ret = RTMP_ConnectStream(rtmp, 0);
        if(!ret) {
            LOGE2("建立流连接失败");
            break;
        }

        //6.记录开始推流的时间
        start_time = RTMP_GetTime();

        //跟服务器连接通了
        packets.setWork(1);
        RTMPPacket *packet = 0;
        while (isStart) {
            int ret = packets.pop(packet);
            if(!isStart) {
                break;
            }
            if(!ret) {
                continue;
            }
            //7.发送数据包
            packet->m_nInfoField2 = rtmp->m_stream_id;
            ret = RTMP_SendPacket(rtmp, packet, 1);
            if(!ret) {
                //rtmp 断开连接
                break;
            }
        }
        ReleaseRTMPPacket(&packet);
    }while (0);

    isStart = 0;
    packets.setWork(0);
    packets.clear();
    if(rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    delete (url);

    return 0; //记得！！！
}

extern "C" JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_startLiveNative(JNIEnv *env, jobject thiz, jstring path_) {
    if(isStart) {
        return;
    }
    isStart = 1;
    const char *path = env->GetStringUTFChars(path_, 0);
    //要进行服务器连接，创建子线程
    char *url = new char[strlen(path) + 1]; //"\0" //处理悬空指针问题
    strcpy(url, path);
    pthread_create(&pid_start, 0, task_start, url);
    env->ReleaseStringUTFChars(path_, path);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_stopLiveNative(JNIEnv *env, jobject thiz) {
    isStart = 0;
    packets.setWork(0);
    pthread_join(pid_start, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_initVideoEncoderNative(JNIEnv *env, jobject thiz,
                                                                 jint width, jint height,
                                                                 jint bitrate, jint fps) {
    if(video_channel) {
        video_channel->initVideoEncoder(width, height, bitrate, fps);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_pushVideoNative(JNIEnv *env, jobject thiz,
                                                          jbyteArray data_) {
    if(!video_channel || !isStart) {
        return;
    }
    jbyte *data = env->GetByteArrayElements(data_, 0);
    video_channel->encodeData(reinterpret_cast<uint8_t *>(data));

    env->ReleaseByteArrayElements(data_, data, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_releaseNative(JNIEnv *env, jobject thiz) {
    DELETE(video_channel);
    DELETE(audio_channel);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_initAudioEncoderNative(JNIEnv *env, jobject thiz,
                                                                 jint sample_rate,
                                                                 jint channels) {
    if(audio_channel) {
        audio_channel->initAudioEncoder(sample_rate, channels);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_livepushclient_LivePusher_pushAudioNative(JNIEnv *env, jobject thiz,
                                                          jbyteArray data_) {
    if(!audio_channel || !isStart) {
        return;
    }
    jbyte *data = env->GetByteArrayElements(data_, 0);
    audio_channel->encodeData(data);
    env->ReleaseByteArrayElements(data_, data, 0);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_sty_ne_livepushclient_LivePusher_getInputSamplesNative(JNIEnv *env, jobject thiz) {
    if(audio_channel) {
        return audio_channel->getInputSamples();
    }
    return -1;
}