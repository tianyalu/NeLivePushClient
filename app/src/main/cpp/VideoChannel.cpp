//
// Created by tian on 2020/4/14.
//

#include "VideoChannel.h"

VideoChannel::VideoChannel() {
    pthread_mutex_init(&mutex, 0);
}

VideoChannel::~VideoChannel() {
    pthread_mutex_destroy(&mutex);
}

/**
 * 初始化x264编码器
 * @param width
 * @param height
 * @param bitrate
 * @param fps
 */
void VideoChannel::initVideoEncoder(int width, int height, int bitrate, int fps) {
    //宽高发生改变时，如果正在编码，避免重复初始化
    pthread_mutex_lock(&mutex);
    mWidth = width;
    mHeight = height;
    mBitrate = bitrate;
    mFps = fps;

    x264_param_t param;
    //ultrafast 最快
    //zerolatency 零延时
    x264_param_default_preset(&param, "ultrafast", "zerolatency");

    //编码规格，参考：https://wikipedia.tw.wjbk.site/wiki/H.264/MPEG-4_AVC  show/h264_rate_frame_cal.png
    // 800*480 25fpx
    //   一帧      宏块大小
    // 800*480 / (16 * 16) * 25 = 37500 （宏块/S）
    param.i_level_idc = 32;
    param.i_width = mWidth;
    param.i_height = mHeight;
    param.i_bframe = 0; //没有B帧
    //ABR：平均码率，CPQ：恒定质量，CRF：恒定码率
    param.rc.i_rc_method = X264_RC_ABR;
    //比特率，单位：kb/s
    param.rc.i_bitrate = mBitrate / 1000;
    //瞬时最大码率
    param.rc.i_vbv_max_bitrate = mBitrate / 100 * 1.2;
    //缓存区
    param.rc.i_vbv_buffer_size = mBitrate / 1000;
    //码率控制不通过时间戳和timebase，而是根据fps
    param.b_vfr_input = 0;
    param.i_fps_num = mFps; //fps 分子
    param.i_fps_den = 1; //fps 分母
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;
    //关键帧距离
    param.i_keyint_max = mFps * 2;
    //是(否)复制sps和pps到每个关键帧前面
    param.b_repeat_headers = 1;
    param.i_threads = 1;
    x264_param_apply_profile(&param, "baseline");

    pic_in = new x264_picture_t;
    //输入图像初始化
    x264_picture_alloc(pic_in, param.i_csp, param.i_width, param.i_height);

    videoEncoder = x264_encoder_open(&param);
    if(videoEncoder) {
        LOGE2("x264编码器打开成功");
    }
    pthread_mutex_unlock(&mutex);
}

void VideoChannel::encodeData(uint8_t *data) {

}

