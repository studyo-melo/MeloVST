#include "opus_decoder.h"

#define TAG "OpusDecoderWrapper"

OpusDecoderWrapper::OpusDecoderWrapper(int sample_rate, int channels, int duration_ms) {
    int error;
    audio_dec_ = opus_decoder_create(sample_rate, channels, &error);
    if (audio_dec_ == nullptr) {
        return;
    }

    frame_size_per_channel_ = sample_rate / 1000 * duration_ms;
    numChannels = channels;
}

OpusDecoderWrapper::~OpusDecoderWrapper() {
    if (audio_dec_ != nullptr) {
        opus_decoder_destroy(audio_dec_);
    }
}

std::vector<int16_t> OpusDecoderWrapper::Decode(std::vector<uint8_t> opus) const {
    std::vector<int16_t> pcm(frame_size_per_channel_*numChannels);
    if (audio_dec_ == nullptr) {
        return pcm;
    }

    // mon paquet fait 1300 bytes
    auto ret = opus_decode(audio_dec_, opus.data(), opus.size(), pcm.data(), frame_size_per_channel_, 0);
    if (ret < 0) {
        throw std::runtime_error("Failed to decode audio err code: " + std::to_string(ret));
    }
    pcm.reserve(ret);

    return pcm;
}

void OpusDecoderWrapper::ResetState() {
    if (audio_dec_ != nullptr) {
        opus_decoder_ctl(audio_dec_, OPUS_RESET_STATE);
    }
}

