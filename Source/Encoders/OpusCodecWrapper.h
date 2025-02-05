#pragma once
#include "opus.h"
#include <vector>
#include <stdexcept>
#include <juce_core/juce_core.h>
#include "aoo/aoo.hpp"
#define MAX_OPUS_PACKET_SIZE 1500

class OpusCodecWrapper {
public:
    OpusCodecWrapper(int opus_sample_rate, int daw_sample_rate, int channels, int duration_ms):
    frameDurationInMs(duration_ms),
    numChannels(channels),
    opusSampleRate(opus_sample_rate),
    dawSampleRate(daw_sample_rate)
    {
        int error;
        source = aoo::isource::create(0);
        encoder = opus_encoder_create(opusSampleRate, channels, OPUS_APPLICATION_VOIP, &error);
        if (error != OPUS_OK)
            throw std::runtime_error("Failed to create Opus encoder: " + std::string(opus_strerror(error)));

        decoder = opus_decoder_create(dawSampleRate, channels, &error);
        if (decoder == nullptr) {
            return;
        }
        // Configuration de l'encodeur
        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(64000));
        // opus_encoder_ctl(encoder, OPUS_SET_BITRATE(OPUS_AUTO));
        // opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(0));
        // opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(5));
        // opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(0));
        // opus_encoder_ctl(encoder, OPUS_SET_DTX(0));
        // opus_decoder_ctl(decoder, OPUS_SET_GAIN(1));

        encoderFrameSizePerChannel = opusSampleRate / 1000 * frameDurationInMs;
        encoder_frame_size_ = opusSampleRate / 1000 * channels * duration_ms;
        decoderFrameSizePerChannel = dawSampleRate / 1000 * frameDurationInMs;
        decoder_frame_size_ = dawSampleRate / 1000 * channels * duration_ms;
    }

    ~OpusCodecWrapper() {
        if (encoder)
            opus_encoder_destroy(encoder);
        if (decoder)
            opus_decoder_destroy(decoder);
    }

    std::vector<unsigned char> encode_float(const std::vector<float>& pcm, int nbSamples) const {
        // On définit une taille maximale pour le paquet de sortie (par exemple 4000 octets)
        std::vector<unsigned char> res(4000);

        // Opus attend nbSamples échantillons par canal.
        int ret = opus_encode_float(encoder, pcm.data(), encoder_frame_size_, res.data(), static_cast<int>(res.size()));
        if (ret < 0) {
            return res;
            //throw std::runtime_error("Encoding failed with error code: " + std::to_string(ret));
        }

        res.resize(ret);
        return res;
    }

    std::vector<float> decode_float(const std::vector<unsigned char>& opus) const {
        // Allocation initiale pour frameSizePerChannel * numChannels échantillons
        std::vector<float> pcm(decoderFrameSizePerChannel * numChannels);
        int ret = opus_decode_float(decoder, opus.data(), opus.size(), pcm.data(), decoder_frame_size_, 0);
        if (ret < 0) {
            return pcm;
            //throw std::runtime_error("Failed to decode audio err code: " + std::to_string(ret));
        }
        // Ret est le nombre d'échantillons par canal, on redimensionne donc pour ret * numChannels échantillons au total
        pcm.resize(ret * numChannels);

        return pcm;
    }

    // Un paquet Opus contient :
    // Un en-tête : qui inclut des informations comme le type de trame, le débit binaire, et la taille des trames.
    // Les données encodées: même pour un silence, Opus insère des données encodées représentant le silence.
    void encode(std::vector<int16_t> pcm, std::function<void(std::vector<unsigned char> opus)> handler) {
        if (encoder == nullptr) {
            return;
        }

        if (in_buffer_.empty()) {
            in_buffer_ = std::move(pcm);
        } else {
            in_buffer_.insert(in_buffer_.end(), pcm.begin(), pcm.end());
        }

        while (in_buffer_.size() >= encoderFrameSizePerChannel * numChannels) {
            std::vector<uint8_t> opus(encoder_frame_size_ * numChannels, 0);
            auto ret = opus_encode(encoder, in_buffer_.data(), encoder_frame_size_, opus.data(), opus.size());
            if (ret < 0) {
                // ESP_LOGE(TAG, "Failed to encode audio, error code: %ld", ret);
                return;
            }
            opus.resize(ret);

            if (handler != nullptr) {
                handler(std::move(opus));
            }

            in_buffer_.erase(in_buffer_.begin(), in_buffer_.begin() + encoder_frame_size_);
        }
    }

    bool decode(std::vector<unsigned char> opus, std::vector<int16_t>& pcm) {
        if (decoder == nullptr) {
            return false;
        }

        pcm.resize(decoder_frame_size_ * numChannels);
        auto ret = opus_decode(decoder, opus.data(), opus.size(), pcm.data(), decoder_frame_size_ * numChannels, 1);
        if (ret < 0) {
            return false;
        }

        return true;
    }

    int getChannels() const {
        return numChannels;
    }

private:
    aoo_source *source = nullptr;
    std::vector<int16_t> in_buffer_;
    OpusEncoder *encoder = nullptr;
    OpusDecoder *decoder = nullptr;
    int encoder_frame_size_;
    int decoder_frame_size_;
    int encoderFrameSizePerChannel;
    int decoderFrameSizePerChannel;
    int frameDurationInMs;
    int numChannels;
    int opusSampleRate;
    int dawSampleRate;
    int maxPacketSize = 4000;
};
