//
// Created by tian on 2020/4/14.
//

#ifndef NELIVEPUSHCLIENT_AUDIOCHANNEL_H
#define NELIVEPUSHCLIENT_AUDIOCHANNEL_H

#include <cstdint>
#include <sys/types.h>
#include <faac.h>
#include <rtmp.h>
#include <cstring>
#include "macro.h"


class AudioChannel {
    typedef void(*AudioCallback)(RTMPPacket *packet);
public:
    AudioChannel();

    ~AudioChannel();

    void initAudioEncoder(int sample_rate, int channels);
    void encodeData(int8_t *data);

    int getInputSamples();

    void setAudioCallback(AudioCallback callback);

    RTMPPacket *getAudioSeqHeader();

private:
    int mChannels;

    u_long inputSamples;
    u_long maxOutputBytes;
    faacEncHandle audioEncoder = 0;
    u_char *buffer = 0;
    AudioCallback audioCallback;
};


#endif //NELIVEPUSHCLIENT_AUDIOCHANNEL_H
