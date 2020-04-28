//
// Created by tian on 2020/4/14.
//




#include "AudioChannel.h"

AudioChannel::AudioChannel() {

}

AudioChannel::~AudioChannel() {
    DELETE(buffer);
    if(audioEncoder) {
        faacEncClose(audioEncoder);
        audioEncoder = 0;
    }
}

void AudioChannel::initAudioEncoder(int sample_rate, int channels) {
    mChannels = channels;
    /**
     * inputSamples ： 编码器每次进行编码时接收的最大样本数
     * maxOutputBytes：编码器最大输出数据个数（字节数）
     */
    audioEncoder = faacEncOpen(sample_rate, channels, &inputSamples, &maxOutputBytes);

    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(audioEncoder);//获取当前编码器的配置

    config->mpegVersion = MPEG4;//使用MPEG-4标准
    config->aacObjectType = LOW;//LC 低复杂度标准
    config->inputFormat = FAAC_INPUT_16BIT;//16位
    config->outputFormat = 0;//需要原始数据，而不是adts
    config->useTns = 1;//降噪
    config->useLfe = 0;//环绕

    int ret = faacEncSetConfiguration(audioEncoder, config);
    if (!ret) {
        LOGE2("音频编码器配置失败");
        return;
    }
    //初始化输出缓冲区
    buffer = new u_char[maxOutputBytes];
}

int AudioChannel::getInputSamples() {
    return inputSamples;
}

void AudioChannel::encodeData(int8_t *data) {
    //faac编码，返回编码后数据的字节长度
    int byteLen = faacEncEncode(audioEncoder, reinterpret_cast<int32_t *>(data), inputSamples, buffer,
                  maxOutputBytes);
    if(byteLen > 0) {
        LOGE2("音频编码成功");
        //组RTMP包
        //参考：https://www.jianshu.com/p/f87ac6aa6d63 音频trmp包.png（此处没有发“解码信息”那一行，经测试不发也行）
        RTMPPacket *packet = new RTMPPacket;
        int body_size = 2 + byteLen;
        RTMPPacket_Alloc(packet, body_size);

        packet->m_body[0] = 0xAF; //双声道
        if(mChannels == 1) {
            packet->m_body[0] = 0xAE; //单声道
        }
        packet->m_body[1] = 0x01;
        memcpy(&packet->m_body[2], buffer, byteLen);

        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nBodySize = body_size;
        packet->m_nTimeStamp = -1;
        packet->m_hasAbsTimestamp = 1;
        packet->m_nChannel = 11;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

        audioCallback(packet);
    }
}

void AudioChannel::setAudioCallback(AudioChannel::AudioCallback callback) {
    this->audioCallback = callback;
}

//该方法没有用到
RTMPPacket *AudioChannel::getAudioSeqHeader() {
    u_char *ppBuffer;
    u_long byteLen;
    faacEncGetDecoderSpecificInfo(audioEncoder, &ppBuffer, &byteLen);

    //组RTMP包
    RTMPPacket *packet = new RTMPPacket();
    int body_size = 2 + byteLen;
    RTMPPacket_Alloc(packet, body_size);

    packet->m_body[0] = 0xAF; //双声道
    if(mChannels == 1) {
        packet->m_body[0] = 0xAE; //单声道
    }

    packet->m_body[1] = 0x00; //序列头配置信息为0x00
    memcpy(&packet->m_body[2], ppBuffer, byteLen);

    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = body_size;
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 1;
    packet->m_nChannel = 11;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

    //audioCallback(packet);

    //return nullptr;
    return packet;
}


