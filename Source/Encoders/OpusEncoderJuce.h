#pragma once
#include <opus.h>
#include <vector>
#include <fstream>
#include <iostream>

class OpusEncoderJUCE {
public:
    OpusEncoderJUCE(int sampleRate, int channels, int bitrate)
        : sampleRate(sampleRate), channels(channels) {
        int error;
        encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_AUDIO, &error);
        if (error != OPUS_OK) {
            std::cerr << "Opus encoder creation failed: " << opus_strerror(error) << std::endl;
            encoder = nullptr;
        }

        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
    }

    ~OpusEncoderJUCE() {
        if (encoder) {
            opus_encoder_destroy(encoder);
        }
    }

    std::vector<unsigned char> processAudioBlock(const std::vector<float> &buffer) {
        if (!encoder || buffer.empty()) return std::vector<unsigned char>(0);

        // Vérification que la taille du buffer est correcte (multiple du nombre de canaux)
        if (buffer.size() % channels != 0) {
            std::cerr << "Invalid buffer size. Must be a multiple of channel count." << std::endl;
            return std::vector<unsigned char>(0);
        }

        int numSamples = buffer.size() / channels;
        std::vector<int16_t> pcmData(buffer.size());

        // Conversion de float (-1.0 à 1.0) vers PCM 16 bits (-32768 à 32767)
        for (size_t i = 0; i < buffer.size(); ++i) {
            pcmData[i] = static_cast<int16_t>(std::clamp(buffer[i] * 32767.0f, -32768.0f, 32767.0f));
        }

        // Encodage avec Opus
        unsigned char opusData[4000]; // Buffer de sortie
        int encodedBytes = opus_encode(encoder, pcmData.data(), numSamples, opusData, sizeof(opusData));
        if (encodedBytes > 0) {
            return std::vector<unsigned char>(opusData, opusData + encodedBytes);
        }
        return std::vector<unsigned char>(0);
    }

private:
    OpusEncoder *encoder = nullptr;
    int sampleRate;
    int channels;
};
