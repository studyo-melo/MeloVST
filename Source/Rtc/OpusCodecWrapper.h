#pragma once
#include <opus.h>
#include <vector>
#include <stdexcept>
#include <cmath>

class OpusCodecWrapper {
public:
    OpusCodecWrapper(int sampleRate, int channels)
        : sampleRate(sampleRate), channels(channels) {
        // Validation des paramètres
        if (sampleRate != 8000 && sampleRate != 12000 && sampleRate != 16000 &&
            sampleRate != 24000 && sampleRate != 48000)
            throw std::invalid_argument("Invalid sample rate for Opus encoder: " + std::to_string(sampleRate));
        if (channels != 1 && channels != 2)
            throw std::invalid_argument("Invalid channel count for Opus encoder: " + std::to_string(channels));

        // Initialisation de l'encodeur
        int error;
        encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_AUDIO, &error);
        if (error != OPUS_OK)
            throw std::runtime_error("Failed to create Opus encoder: " + std::string(opus_strerror(error)));

        // Configuration de l'encodeur
        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(OPUS_AUTO));
        opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(10));
        opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10));
        opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1));
        opus_encoder_ctl(encoder, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND));

        // Initialisation du décodeur
        decoder = opus_decoder_create(sampleRate, channels, &error);
        if (error != OPUS_OK)
            throw std::runtime_error("Failed to create Opus decoder: " + std::string(opus_strerror(error)));
    }

    ~OpusCodecWrapper() {
        // Libération des ressources
        if (encoder)
            opus_encoder_destroy(encoder);
        if (decoder)
            opus_decoder_destroy(decoder);
    }

    std::vector<uint8_t> encode(const float* pcmData, int frameSize) {
        if (!encoder)
            throw std::runtime_error("Encoder not initialized");

        std::vector<uint8_t> encodedData(4000); // Taille maximale pour l'encodage
        int bytes = opus_encode_float(encoder, pcmData, frameSize, encodedData.data(), static_cast<int>(encodedData.size()));
        if (bytes < 0)
            throw std::runtime_error("Opus encoding error: " + std::string(opus_strerror(bytes)));

        encodedData.resize(bytes); // Ajuster la taille au nombre d'octets réellement utilisés
        return encodedData;
    }

    std::vector<float> decode(const std::vector<uint8_t>& encodedData, int frameSize) {
        if (!decoder)
            throw std::runtime_error("Decoder not initialized");

        std::vector<float> pcmData(frameSize * channels); // Préparation du tampon de sortie
        int decodedSamples = opus_decode_float(decoder, encodedData.data(), static_cast<int>(encodedData.size()), pcmData.data(), frameSize, 0);
        if (decodedSamples < 0)
            throw std::runtime_error("Opus decoding error: " + std::string(opus_strerror(decodedSamples)));

        pcmData.resize(decodedSamples * channels); // Ajuster la taille au nombre d'échantillons décodés
        return pcmData;
    }

private:
    int sampleRate;
    int channels;
    OpusEncoder* encoder = nullptr;
    OpusDecoder* decoder = nullptr;
};
