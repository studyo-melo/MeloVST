#include "opus_decoder.h"

#define TAG "OpusDecoderWrapper"

OpusDecoderWrapper::OpusDecoderWrapper(int sample_rate, int channels, int duration_ms) {
    int error;
    audio_dec_ = opus_decoder_create(sample_rate, channels, &error);
    if (audio_dec_ == nullptr) {
        return;
    }

    frame_size_ = sample_rate / 1000 * channels * duration_ms;
}

OpusDecoderWrapper::~OpusDecoderWrapper() {
    if (audio_dec_ != nullptr) {
        opus_decoder_destroy(audio_dec_);
    }
}

bool OpusDecoderWrapper::Decode(std::vector<uint8_t>&& opus, std::vector<int16_t>& pcm) {
    if (audio_dec_ == nullptr) {
        return false;
    }

    pcm.resize(frame_size_);
    auto ret = opus_decode(audio_dec_, opus.data(), opus.size(), pcm.data(), pcm.size(), 0);
    if (ret < 0) {
        return false;
    }

    return true;
}

void OpusDecoderWrapper::ResetState() {
    if (audio_dec_ != nullptr) {
        opus_decoder_ctl(audio_dec_, OPUS_RESET_STATE);
    }
}

