#pragma once
#include "opus.h"
#include <vector>
#include <stdexcept>
#include <juce_core/juce_core.h>

#define MAX_OPUS_PACKET_SIZE 1500

class OpusCodecWrapper {
public:
    OpusCodecWrapper(int frameDurationInMs, int numChannels, int sampleRate): frameDurationInMs(frameDurationInMs), numChannels(numChannels), sampleRate(sampleRate) {
        int error;
        encoder = opus_encoder_create(sampleRate, numChannels, OPUS_APPLICATION_VOIP, &error);
        if (error != OPUS_OK)
            throw std::runtime_error("Failed to create Opus encoder: " + std::string(opus_strerror(error)));

        decoder = opus_decoder_create(sampleRate, numChannels, &error);
        if (decoder == nullptr) {
            return;
        }
        // Configuration de l'encodeur
        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(OPUS_AUTO));
        opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(10));
        opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(5));
        opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(0));
        opus_encoder_ctl(encoder, OPUS_SET_DTX(1));
        opus_decoder_ctl(decoder, OPUS_SET_GAIN(0));

        frameSizePerChannel = sampleRate / 1000 * frameDurationInMs;
    }

    ~OpusCodecWrapper() {
        if (encoder)
            opus_encoder_destroy(encoder);
        if (decoder)
            opus_decoder_destroy(decoder);
    }

    // Un paquet Opus contient :
    // Un en-tête : qui inclut des informations comme le type de trame, le débit binaire, et la taille des trames.
    // Les données encodées: même pour un silence, Opus insère des données encodées représentant le silence.
    std::vector<int8_t> encode(std::vector<int16_t> pcm) {
        std::vector<int8_t> res;
        int frame_size_ = frameSizePerChannel * numChannels;
        if (encoder == nullptr) {
            return res;
        }
        if (in_buffer_.empty()) {
            in_buffer_ = std::move(pcm);
        } else {
            in_buffer_.insert(in_buffer_.end(), pcm.begin(), pcm.end());
        }

        while (in_buffer_.size() >= frame_size_) {
            std::vector<int8_t> opus(MAX_OPUS_PACKET_SIZE);
            auto ret = opus_encode(encoder, pcm.data(), frameSizePerChannel, reinterpret_cast<unsigned char *>(opus.data()), opus.size());
            if (ret < 0) {
                in_buffer_.erase(in_buffer_.begin(), in_buffer_.begin() + frame_size_);
                return opus;
            }
            opus.resize(ret);
            res.assign(opus.begin(), opus.end());

            in_buffer_.erase(in_buffer_.begin(), in_buffer_.begin() + frame_size_);
        }
        return res;
    }

    std::vector<int8_t> encode_in_place(std::vector<int16_t> pcm) const {
        std::vector<int8_t> opus(MAX_OPUS_PACKET_SIZE);
        auto ret = opus_encode(encoder, pcm.data(), frameSizePerChannel, reinterpret_cast<unsigned char *>(opus.data()), opus.size());
        if (ret < 0) {
            throw std::runtime_error("Failed to encode audio err code: " + std::to_string(ret));
        }
        opus.resize(ret);
        return opus;
    }

    std::vector<int16_t> decode(std::vector<int8_t> opus) const {
        std::vector<int16_t> pcm(frameSizePerChannel * numChannels);
        if (decoder == nullptr) {
            return pcm;
        }

        // mon paquet fait 1300 bytes
        auto ret = opus_decode(decoder, reinterpret_cast<const unsigned char *>(opus.data()), opus.size(), pcm.data(), frameSizePerChannel, 0);
        if (ret < 0) {
            throw std::runtime_error("Failed to decode audio err code: " + std::to_string(ret));
        }
        pcm.reserve(ret);

        return pcm;
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
    int frameSizePerChannel;
    int frameDurationInMs;
    int numChannels;
    int sampleRate;
};
