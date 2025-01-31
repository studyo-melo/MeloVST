#pragma once
#include "opus.h"
#include <vector>
#include <stdexcept>
#include <juce_core/juce_core.h>

#define MAX_OPUS_PACKET_SIZE 1500

class OpusCodecWrapper {
public:
    OpusCodecWrapper(int sample_rate, int channels, int duration_ms): frameDurationInMs(duration_ms), numChannels(channels), sampleRate(sample_rate) {
        int error;
        encoder = opus_encoder_create(sample_rate, channels, OPUS_APPLICATION_VOIP, &error);
        if (error != OPUS_OK)
            throw std::runtime_error("Failed to create Opus encoder: " + std::string(opus_strerror(error)));

        decoder = opus_decoder_create(sample_rate, channels, &error);
        if (decoder == nullptr) {
            return;
        }
        // Configuration de l'encodeur
        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(OPUS_AUTO));
        opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(0));
        opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(5));
        opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(0));
        opus_encoder_ctl(encoder, OPUS_SET_DTX(0));
        opus_decoder_ctl(decoder, OPUS_SET_GAIN(1));

        frameSizePerChannel = sampleRate / 1000 * frameDurationInMs;
        frame_size_ = sample_rate / 1000 * channels * duration_ms;
    }

    ~OpusCodecWrapper() {
        if (encoder)
            opus_encoder_destroy(encoder);
        if (decoder)
            opus_decoder_destroy(decoder);
    }

    std::vector<unsigned char> encode_float(std::vector<float> pcm) const {
        std::vector<unsigned char> res(frameSizePerChannel);
        auto ret = opus_encode_float(encoder, pcm.data(), frameSizePerChannel, res.data(), pcm.size());
        if (ret < 0) {
            throw std::runtime_error("Encoding failed with error code: " + std::to_string(ret));
        }
        res.resize(ret);
        return res;
    }

    std::vector<float> decode_float(const std::vector<unsigned char> &opus) const {
        std::vector<float> pcm(frameSizePerChannel * numChannels);
        auto ret = opus_decode_float(decoder, opus.data(), opus.size(), pcm.data(), frameSizePerChannel, 0);
        if (ret < 0) {
            throw std::runtime_error("Failed to decode audio err code: " + std::to_string(ret));
        }
        pcm.resize(ret);

        return pcm;
    }

    // Un paquet Opus contient :
    // Un en-tête : qui inclut des informations comme le type de trame, le débit binaire, et la taille des trames.
    // Les données encodées: même pour un silence, Opus insère des données encodées représentant le silence.
    void encode(std::vector<int16_t>&& pcm, std::function<void(std::vector<uint8_t> opus)> handler) {
        if (encoder == nullptr) {
            return;
        }

        if (in_buffer_.empty()) {
            in_buffer_ = std::move(pcm);
        } else {
            in_buffer_.insert(in_buffer_.end(), pcm.begin(), pcm.end());
        }

        while (in_buffer_.size() >= frame_size_) {
            std::vector<uint8_t> opus(MAX_OPUS_PACKET_SIZE);
            auto ret = opus_encode(encoder, in_buffer_.data(), frame_size_, opus.data(), opus.size());
            if (ret < 0) {
                // ESP_LOGE(TAG, "Failed to encode audio, error code: %ld", ret);
                return;
            }
            opus.resize(ret);

            if (handler != nullptr) {
                handler(std::move(opus));
            }

            in_buffer_.erase(in_buffer_.begin(), in_buffer_.begin() + frame_size_);
        }
    }

    bool decode(std::vector<uint8_t>&& opus, std::vector<int16_t>& pcm) {
        if (decoder == nullptr) {
            return false;
        }

        pcm.resize(frame_size_);
        auto ret = opus_decode(decoder, opus.data(), opus.size(), pcm.data(), pcm.size(), 0);
        if (ret < 0) {
            return false;
        }

        return true;
    }

    int getFrameSize() const {
        return static_cast<int>(frameDurationInMs * sampleRate);
    }

    int getSampleRate() const {
        return sampleRate;
    }

    int getChannels() const {
        return numChannels;
    }

private:
    std::vector<int16_t> in_buffer_;
    OpusEncoder *encoder = nullptr;
    OpusDecoder *decoder = nullptr;
    int frame_size_;
    int frameSizePerChannel;
    int frameDurationInMs;
    int numChannels;
    int sampleRate;
    int maxPacketSize = 4000;
};
