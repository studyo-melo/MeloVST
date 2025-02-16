#pragma once
#include <opus.h>
#include <vector>
#include <stdexcept>
#include <functional>

#define MAX_OPUS_PACKET_SIZE 1500

class OpusCodecWrapper {
public:
    OpusCodecWrapper(const int sample_rate, const int channels, const int duration_ms, const int bitrate): frameDurationInMs(duration_ms), numChannels(channels), sampleRate(sample_rate) {
        int error;
        encoder = opus_encoder_create(sample_rate, channels, OPUS_APPLICATION_VOIP, &error);
        if (error != OPUS_OK)
            throw std::runtime_error("Failed to create Opus encoder: " + std::string(opus_strerror(error)));

        decoder = opus_decoder_create(sample_rate, channels, &error);
        if (decoder == nullptr) {
            return;
        }
        // Configuration de l'encodeur
        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
        // opus_encoder_ctl(encoder, OPUS_SET_BITRATE(OPUS_AUTO));
        // opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(0));
        // opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(5));
        // opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(0));
        // opus_encoder_ctl(encoder, OPUS_SET_DTX(0));
        // opus_decoder_ctl(decoder, OPUS_SET_GAIN(1));

        frameSizePerChannel = sampleRate / 1000 * frameDurationInMs;
        frame_size_ = sample_rate / 1000 * channels * duration_ms;
    }

    ~OpusCodecWrapper() {
        if (encoder)
            opus_encoder_destroy(encoder);
        if (decoder)
            opus_decoder_destroy(decoder);
    }

    std::vector<unsigned char> encode_float(const std::vector<float>& pcm, const int nbSamples) const {
        // On définit une taille maximale pour le paquet de sortie (par exemple 4000 octets)
        std::vector<unsigned char> res(4000);

        // Opus attend nbSamples échantillons par canal.
        const int ret = opus_encode_float(encoder, pcm.data(), nbSamples, res.data(), static_cast<int>(res.size()));
        if (ret < 0) {
            throw std::runtime_error("Encoding failed with error code: " + std::to_string(ret));
        }

        res.resize(ret);
        return res;
    }

    std::vector<float> decode_float(const std::vector<unsigned char>& opus) const {
        // Allocation initiale pour frameSizePerChannel * numChannels échantillons
        std::vector<float> pcm(frameSizePerChannel * numChannels);
        const int ret = opus_decode_float(decoder, opus.data(), opus.size(), pcm.data(), frameSizePerChannel, 0);
        if (ret < 0) {
            return pcm;
        }
        // Ret est le nombre d'échantillons par canal, on redimensionne donc pour ret * numChannels échantillons au total
         pcm.resize(ret * numChannels);

        return pcm;
    }

    // Un paquet Opus contient :
    // Un en-tête : qui inclut des informations comme le type de trame, le débit binaire, et la taille des trames.
    // Les données encodées: même pour un silence, Opus insère des données encodées représentant le silence.
    void encode(std::vector<int16_t> pcm, const std::function<void(std::vector<unsigned char> opus)> &handler) {
        if (encoder == nullptr) {
            return;
        }

        if (in_buffer_.empty()) {
            in_buffer_ = std::move(pcm);
        } else {
            in_buffer_.insert(in_buffer_.end(), pcm.begin(), pcm.end());
        }

        while (in_buffer_.size() >= frame_size_ * numChannels) {
            std::vector<uint8_t> opus(frame_size_ * numChannels, 0);
            const auto ret = opus_encode(encoder, in_buffer_.data(), frame_size_, opus.data(), opus.size());
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

    bool decode(const std::vector<unsigned char> &opus, std::vector<int16_t>& pcm) const {
        if (decoder == nullptr) {
            return false;
        }

        pcm.resize(frame_size_ * numChannels);
        if (const auto ret = opus_decode(decoder, opus.data(), opus.size(), pcm.data(), frame_size_ * numChannels, 1); ret < 0) {
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
    std::vector<float> in_buffer_float_;
    OpusEncoder *encoder = nullptr;
    OpusDecoder *decoder = nullptr;
    int frame_size_;
    int frameSizePerChannel;
    int frameDurationInMs;
    int numChannels;
    int sampleRate;
};
