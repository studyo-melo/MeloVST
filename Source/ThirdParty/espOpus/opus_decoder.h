#ifndef _OPUS_DECODER_WRAPPER_H_
#define _OPUS_DECODER_WRAPPER_H_

#include <functional>
#include <vector>
#include <cstdint>

#include "opus.h"


class OpusDecoderWrapper {
public:
    OpusDecoderWrapper(int sample_rate, int channels, int duration_ms = 60);
    ~OpusDecoderWrapper();

    std::vector<int16_t> Decode(std::vector<uint8_t> opus) const;
    void ResetState();

private:
    struct OpusDecoder* audio_dec_ = nullptr;
    int frame_size_per_channel_;
    int numChannels;
};

#endif // _OPUS_DECODER_WRAPPER_H_
