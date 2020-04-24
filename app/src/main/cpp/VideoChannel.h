//
// Created by tian on 2020/4/14.
//

#ifndef NELIVEPUSHCLIENT_VIDEOCHANNEL_H
#define NELIVEPUSHCLIENT_VIDEOCHANNEL_H

#include <pthread.h>
#include <x264.h>
#include <cstring>
#include <rtmp.h>
#include "macro.h"

class VideoChannel {
    typedef void(*VideoCallback)(RTMPPacket *packet);
public:
    VideoChannel();

    virtual ~VideoChannel();

    void initVideoEncoder(int width, int height, int bitrate, int fps);


    void encodeData(uint8_t *data);

    void setVideoCallback(VideoCallback callback);

private:
    pthread_mutex_t mutex;
    int mWidth;
    int mHeight;
    int mBitrate;
    int mFps;
    x264_t *videoEncoder = 0;
    x264_picture_t *pic_in = 0;
    int y_len;
    int uv_len;
    VideoCallback videoCallback;
    void senSpsPps(uint8_t sps[100], uint8_t pps[100], int sps_len, int pps_len);

    void sendFrame(int type, uint8_t *payload, int iPayload);
};


#endif //NELIVEPUSHCLIENT_VIDEOCHANNEL_H
