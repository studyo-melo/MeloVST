#pragma once
#include <fstream>
#include <cmath>
#include <opus.h>

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

// Fonction pour écrire les données PCM dans un fichier WAV
inline void writeWavFile(const std::string& filename, const std::vector<float>& pcmData, int sampleRate, int channels)
{
    // Paramètres WAV
    int byteRate = sampleRate * channels * sizeof(int16_t); // 16 bits par échantillon
    int blockAlign = channels * sizeof(int16_t);

    // Calcul de la taille des données
    int dataSize = pcmData.size() * sizeof(int16_t);

    // Ouverture du fichier WAV
    std::ofstream wavFile(filename, std::ios::binary);
    if (!wavFile)
        throw std::runtime_error("Failed to open WAV file for writing");

    // Écriture de l'entête WAV
    wavFile.write("RIFF", 4);
    int chunkSize = 36 + dataSize;
    wavFile.write(reinterpret_cast<const char*>(&chunkSize), 4);
    wavFile.write("WAVE", 4);

    // Format
    wavFile.write("fmt ", 4);
    int subchunk1Size = 16;
    wavFile.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
    int audioFormat = 1; // PCM
    wavFile.write(reinterpret_cast<const char*>(&audioFormat), 2);
    wavFile.write(reinterpret_cast<const char*>(&channels), 2);
    wavFile.write(reinterpret_cast<const char*>(&sampleRate), 4);
    wavFile.write(reinterpret_cast<const char*>(&byteRate), 4);
    wavFile.write(reinterpret_cast<const char*>(&blockAlign), 2);
    int bitsPerSample = 16;
    wavFile.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    // Data
    wavFile.write("data", 4);
    wavFile.write(reinterpret_cast<const char*>(&dataSize), 4);

    // Conversion float -> int16_t
    for (float sample : pcmData)
    {
        int16_t intSample = static_cast<int16_t>(std::clamp(sample * 32767.0f, -32768.0f, 32767.0f));
        wavFile.write(reinterpret_cast<const char*>(&intSample), sizeof(int16_t));
    }

    wavFile.close();
    std::cout << "WAV file written: " << filename << std::endl;
}
