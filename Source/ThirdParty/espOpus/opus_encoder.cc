#include "opus_encoder.h"

#include <iostream>
#include <opus.h>

#define TAG "OpusEncoderWrapper"

OpusEncoderWrapper::OpusEncoderWrapper(int sample_rate, int channels, int duration_ms) {
    int error;
    audio_enc_ = opus_encoder_create(sample_rate, channels, OPUS_APPLICATION_VOIP, &error);
    if (audio_enc_ == nullptr) {
        return;
    }
    opus_encoder_ctl(audio_enc_, OPUS_SET_BITRATE(96000));
    opus_encoder_ctl(audio_enc_, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND));
    opus_encoder_ctl(audio_enc_, OPUS_SET_COMPLEXITY(10));

    // Default DTX enabled
    SetDtx(true);
    // Complexity 5 almost uses up all CPU of ESP32C3
    SetComplexity(5);

    frame_size_per_channel_ = sample_rate / 1000 * duration_ms;
    num_channel_ = channels;
}

OpusEncoderWrapper::~OpusEncoderWrapper() {
    if (audio_enc_ != nullptr) {
        opus_encoder_destroy(audio_enc_);
    }
}

void OpusEncoderWrapper::Encode(std::vector<int16_t>&& pcm, std::function<void(std::vector<uint8_t>&& opus)> handler) {
    int frame_size_ = frame_size_per_channel_ * num_channel_;
    if (audio_enc_ == nullptr) {
        return;
    }
    if (in_buffer_.empty()) {
        in_buffer_ = std::move(pcm);
    } else {
        in_buffer_.insert(in_buffer_.end(), pcm.begin(), pcm.end());
    }

    while (in_buffer_.size() >= frame_size_) {
        std::vector<uint8_t> opus(MAX_OPUS_PACKET_SIZE);
        auto ret = opus_encode(audio_enc_, pcm.data(), frame_size_per_channel_, opus.data(), opus.size());
        if (ret < 0) {
            in_buffer_.erase(in_buffer_.begin(), in_buffer_.begin() + frame_size_);
            return;
        }
        opus.resize(ret);

        if (handler != nullptr) {
            handler(std::move(opus));
        }

        in_buffer_.erase(in_buffer_.begin(), in_buffer_.begin() + frame_size_);
    }
}

void OpusEncoderWrapper::ResetState() {
    if (audio_enc_ != nullptr) {
        opus_encoder_ctl(audio_enc_, OPUS_RESET_STATE);
        in_buffer_.clear();
    }
}

void OpusEncoderWrapper::SetDtx(bool enable) {
    if (audio_enc_ != nullptr) {
        opus_encoder_ctl(audio_enc_, OPUS_SET_DTX(enable ? 1 : 0));
    }
}

void OpusEncoderWrapper::SetComplexity(int complexity) {
    if (audio_enc_ != nullptr) {
        opus_encoder_ctl(audio_enc_, OPUS_SET_COMPLEXITY(complexity));
    }
}
