#pragma once
#include <opus.h>

class OpusEncoderWrapper
{
public:
    OpusEncoderWrapper(int sampleRate, int channels, int bitrate)
    {
        int error;
        encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_AUDIO, &error);
        if (error != OPUS_OK)
            throw std::runtime_error("Failed to create Opus encoder");

        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
        opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(10));
    }

    ~OpusEncoderWrapper()
    {
        opus_encoder_destroy(encoder);
    }

    std::vector<uint8_t> encode(const float* pcmData, int frameSize)
    {
        std::vector<uint8_t> encodedData(4000); // Taille maximale possible pour Opus
        int bytes = opus_encode_float(encoder, pcmData, frameSize, encodedData.data(), encodedData.size());
        if (bytes < 0)
            throw std::runtime_error("Opus encoding error");

        encodedData.resize(bytes);
        return encodedData;
    }

private:
    OpusEncoder* encoder;
};