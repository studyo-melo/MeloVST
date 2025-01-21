#pragma once
#include <fstream>
#include <cmath>
#include <opus.h>

#include "../Utils/AudioSettings.h"

class OpusDecoderWrapper
{
public:
    OpusDecoderWrapper(int sampleRate, int channels)
    {
        // Validation des paramètres
        if (sampleRate != 8000 && sampleRate != 12000 && sampleRate != 16000 &&
            sampleRate != 24000 && sampleRate != 48000)
            throw std::invalid_argument("Invalid sample rate for Opus decoder");
        if (channels != 1 && channels != 2)
            throw std::invalid_argument("Invalid channel count for Opus decoder");

        int error;
        decoder = opus_decoder_create(sampleRate, channels, &error);
        if (error != OPUS_OK)
            throw std::runtime_error("Failed to create Opus decoder");

        this->sampleRate = sampleRate;
        this->channels = channels;
    }

    ~OpusDecoderWrapper()
    {
        opus_decoder_destroy(decoder);
    }

    std::vector<float> decode(const std::vector<uint8_t>& rtpPacket) const {
        // Extraction des données audio depuis le paquet RTP
        if (rtpPacket.size() <= sizeof(RTPHeader))
            throw std::runtime_error("Invalid RTP packet size");

        const uint8_t* audioData = rtpPacket.data() + sizeof(RTPHeader);
        size_t audioDataSize = rtpPacket.size() - sizeof(RTPHeader);

        // Buffer pour les données PCM décodées
        std::vector<float> pcmData(960 * channels); // 960 frames max par canal pour Opus à 48 kHz

        // Décodage
        int frameSize = opus_decode_float(decoder, audioData, audioDataSize, pcmData.data(), pcmData.size() / channels, 0);
        if (frameSize < 0)
            throw std::runtime_error("Opus decoding error: " + std::to_string(frameSize));

        // Ajuster la taille du tampon en fonction de la taille des données décodées
        pcmData.resize(frameSize * channels);
        return pcmData;
    }

private:
    OpusDecoder* decoder;
    int sampleRate;
    int channels;
};


