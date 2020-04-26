//
// Created by tian on 2020/4/14.
//


#include "AudioChannel.h"

AudioChannel::AudioChannel() {

}

AudioChannel::~AudioChannel() {

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

}


