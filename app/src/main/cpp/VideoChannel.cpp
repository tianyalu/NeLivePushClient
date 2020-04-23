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

    y_len = width * height;
    uv_len = y_len / 4;

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
    if (videoEncoder) {
        LOGE2("x264编码器打开成功");
    }
    pthread_mutex_unlock(&mutex);
}

void VideoChannel::encodeData(uint8_t *data) {
    pthread_mutex_lock(&mutex);
    //参考：https://www.jianshu.com/p/a2c09daee428    show/yuv_sequence.png
    memcpy(pic_in->img.plane[0], data, y_len);
    for (int i = 0; i < uv_len; ++i) {
        *(pic_in->img.plane[1] + i) = *(data + y_len + 2 * i + 1); //u分量
        *(pic_in->img.plane[2] + i) = *(data + y_len + 2 * i); //v分量
    }
    x264_nal_t *nals = 0;  //NAL: Network Abstract Layer(U: unit)
    int pi_nal;
    x264_picture_t pic_out;
    int ret = x264_encoder_encode(videoEncoder, &nals, &pi_nal, pic_in, &pic_out);
    if(ret < 0) {
        LOGE2("x264编码失败");
        pthread_mutex_unlock(&mutex);
        return;
    }
    //参考：https://www.jianshu.com/p/0c882eca979c
    //SPS：序列参数集，作用于一系列连续的编码图像。
    //PPS：图像参数集，作用于编码视频序列中一个或多个独立的图像。
    //IDR：一个序列的第一个图像叫做 IDR 图像（立即刷新图像），IDR 图像都是 I 帧图像。
    //SPS 和 PPS 包含了初始化H.264解码器所需要的信息参数。
    int sps_len, pps_len;
    uint8_t sps[100];
    uint8_t pps[100];

    for (int i = 0; i < pi_nal; ++i) {
        if(nals[i].i_type == NAL_SPS) {
            sps_len = nals[i].i_payload - 4; //去掉起始码
            memcpy(sps, nals[i].p_payload + 4, sps_len);
        }else if(nals[i].i_type == NAL_PPS) {
            pps_len = nals[i].i_payload - 4; //去掉起始码
            memcpy(pps, nals[i].p_payload + 4, pps_len);
            //sps 和 pps 的发送
            senSpsPps(sps, pps, sps_len, pps_len);
        }else {

        }
    }
    
    pthread_mutex_unlock(&mutex);
}

void VideoChannel::senSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len) {
    //组装RTMPPacket包 参考：https://www.jianshu.com/p/0c882eca979c  视频包.png,视频解码序列包.png
    RTMPPacket *packet = new RTMPPacket;
    int body_size = 5 + 8 + sps_len + 3 + pps_len;
    RTMPPacket_Alloc(packet, body_size);
    int i = 0;
    packet->m_body[i++] = 0x17;

    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;

    packet->m_body[i++] = 0x01;

    packet->m_body[i++] = sps[1];
    packet->m_body[i++] = sps[2];
    packet->m_body[i++] = sps[3];

    packet->m_body[i++] = 0xFF;
    packet->m_body[i++] = 0xE1;

    packet->m_body[i++] = (sps_len >> 8) & 0xFF;
    packet->m_body[i++] = sps_len & 0xFF;

    memcpy(&packet->m_body[i], sps, sps_len);
    i+= sps_len; //注意i要移位

    packet->m_body[i++] = 0x01;

    packet->m_body[i++] = (pps_len >> 8) & 0xFF;
    packet->m_body[i++] = pps_len & 0xFF;

    memcpy(&packet->m_body[i], pps, pps_len);

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = body_size;
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;
    packet->m_nChannel = 10; //通道ID
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
}

