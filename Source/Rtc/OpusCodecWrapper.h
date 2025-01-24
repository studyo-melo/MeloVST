#pragma once
#include <opus.h>
#include <vector>
#include <stdexcept>
#include <juce_core/juce_core.h>



class OpusCodecWrapper {
public:
    OpusCodecWrapper()
        : sampleRate(48000), channels(2) {
        int error;
        encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_AUDIO, &error);
        if (error != OPUS_OK)
            throw std::runtime_error("Failed to create Opus encoder: " + std::string(opus_strerror(error)));

        decoder = opus_decoder_create(sampleRate, channels, &error);
        if (error != OPUS_OK)
            throw std::runtime_error("Failed to create Opus decoder: " + std::string(opus_strerror(error)));

        // Configuration de l'encodeur
        // opus_encoder_ctl(encoder, OPUS_SET_BITRATE(OPUS_AUTO));
        // opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(10));
        // opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10));
        // opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1));
        // opus_encoder_ctl(encoder, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND));
    }

    ~OpusCodecWrapper() {
        if (encoder)
            opus_encoder_destroy(encoder);
        if (decoder)
            opus_decoder_destroy(decoder);
    }

    std::vector<uint8_t> encode(const float* pcmData) {
        if (!encoder)
            throw std::runtime_error("Encoder not initialized");

        std::vector<uint8_t> encodedData(4000); // Taille maximale pour l'encodage
        int frameSize = static_cast<int>(frameDuration * sampleRate);
        int bytes = opus_encode_float(encoder, pcmData, frameSize, encodedData.data(), static_cast<int>(encodedData.size()));
        if (bytes < 0)
            throw std::runtime_error("Opus encoding error: " + std::string(opus_strerror(bytes)));

        encodedData.resize(bytes); // Ajuster la taille au nombre d'octets réellement utilisés
        return encodedData;
    }

    std::vector<float> decode(const std::vector<uint8_t>& encodedData) {
        if (!decoder)
            throw std::runtime_error("Decoder not initialized");

        int frameSize = static_cast<int>(frameDuration * sampleRate);
        std::vector<float> pcmData(frameSize * channels); // Préparation du tampon de sortie
        int decodedSamples = opus_decode_float(decoder, encodedData.data(), static_cast<int>(encodedData.size()), pcmData.data(), frameSize, 0);
        if (decodedSamples <= 0) {
            throw std::runtime_error("Opus decoding error: " + std::string(opus_strerror(decodedSamples)));
        }

        pcmData.resize(decodedSamples * channels); // Ajuster la taille au nombre d'échantillons décodés
        return pcmData;
    }

private:
    int sampleRate;
    int channels;
    double frameDuration = 0.02;
    OpusEncoder* encoder = nullptr;
    OpusDecoder* decoder = nullptr;
};
