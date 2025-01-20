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
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <cstdint>

inline std::string getFilePath(const std::string& filename) {
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        throw std::runtime_error("Impossible de localiser le répertoire personnel (HOME).");
    }

    std::string desktopPath = std::string(homeDir) + "/Desktop";
    return desktopPath + "/" + filename;
}

inline std::fstream initializeWavFile(const std::string& filename, int sampleRate, int channels, int samplePerBlock) {
    std::string filepath = getFilePath(filename);

    std::fstream wavFile(filepath, std::ios::binary | std::ios::out);
    if (!wavFile) {
        throw std::runtime_error("Failed to create WAV file.");
    }

    juce::Logger::outputDebugString("Initializing WAV file with " + std::to_string(sampleRate) + "Hz, " + std::to_string(channels) + " channels");
    // Paramètres pour l'entête initial
    int byteRate = sampleRate * channels * sizeof(int16_t);
    int blockAlign = channels * sizeof(int16_t);
    int dataSize = 0; // Initialement vide
    int chunkSize = 36 + dataSize;

    // Écrire un en-tête provisoire
    wavFile.write("RIFF", 4);
    wavFile.write(reinterpret_cast<const char*>(&chunkSize), 4);
    wavFile.write("WAVE", 4);
    wavFile.write("fmt ", 4);

    int subchunk1Size = 16; // PCM
    int audioFormat = 1;    // Linear PCM
    wavFile.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
    wavFile.write(reinterpret_cast<const char*>(&audioFormat), 2);
    wavFile.write(reinterpret_cast<const char*>(&channels), 2);
    wavFile.write(reinterpret_cast<const char*>(&sampleRate), 4);
    wavFile.write(reinterpret_cast<const char*>(&byteRate), 4);
    wavFile.write(reinterpret_cast<const char*>(&blockAlign), 2);

    int bitsPerSample = 16;
    wavFile.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
    wavFile.write("data", 4);
    wavFile.write(reinterpret_cast<const char*>(&dataSize), 4);

    std::cout << "Initialized WAV file: " << filepath << std::endl;
    return wavFile;
}

inline void appendWavData(const std::string& filename, const std::vector<float>& pcmData) {
    std::string filepath = getFilePath(filename);
    std::ofstream wavFile(filepath, std::ios::binary | std::ios::app);
    if (!wavFile) {
        throw std::runtime_error("Impossible d'ouvrir le fichier WAV pour ajout.");
    }

    for (float sample : pcmData) {
        if (sample < -1.0f || sample > 1.0f) {
            std::cerr << "Sample out of range: " << sample << std::endl;
        }
        int16_t intSample = static_cast<int16_t>(std::clamp(sample * 32767.0f, -32768.0f, 32767.0f));
        wavFile.write(reinterpret_cast<const char*>(&intSample), sizeof(int16_t));
    }

    wavFile.close();
}

inline void finalizeWavFile(std::fstream wavFile) {
    // Aller à la fin pour calculer la taille des données
    wavFile.seekp(0, std::ios::end);
    int fileSize = wavFile.tellp();
    int dataSize = fileSize - 44; // Taille des données (taille totale - en-tête)

    // Mettre à jour la taille totale du fichier
    wavFile.seekp(4, std::ios::beg);
    int chunkSize = fileSize - 8; // Taille totale - 8 octets
    wavFile.write(reinterpret_cast<const char*>(&chunkSize), 4);

    // Mettre à jour la taille des données
    wavFile.seekp(40, std::ios::beg);
    wavFile.write(reinterpret_cast<const char*>(&dataSize), 4);

    wavFile.close();
    std::cout << "Finalized WAV file" << std::endl;
}

