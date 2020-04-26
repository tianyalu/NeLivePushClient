//
// Created by tian on 2020/4/14.
//

#ifndef NELIVEPUSHCLIENT_AUDIOCHANNEL_H
#define NELIVEPUSHCLIENT_AUDIOCHANNEL_H

#include <cstdint>

class AudioChannel {

public:
    AudioChannel();

    ~AudioChannel();

    void initAudioEncoder(int sample_rate, int channels);
    void encodeData(int8_t *data);

private:

};


#endif //NELIVEPUSHCLIENT_AUDIOCHANNEL_H
