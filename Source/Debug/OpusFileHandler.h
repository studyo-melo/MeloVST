#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <juce_core/juce_core.h>
#include <opus.h>

class OpusFileHandler {
public:
    OpusFileHandler(uint32_t sampleRate, int bitrate, int16_t numChannels) : file(nullptr), sampleRate(sampleRate), bitrate(bitrate), numChannels(numChannels), encoder(nullptr) {}
    static std::string getFilePath(const std::string& filename) {
        const char* homeDir = getenv("HOME");
        if (!homeDir) {
            throw std::runtime_error("Impossible de localiser le répertoire personnel (HOME).");
        }

        return std::string(homeDir) + "/Desktop/" + filename;
    }

    bool create(const std::string& filename) {
        std::string filepath = getFilePath(filename);
        std::remove(filepath.c_str());
        juce::Logger::outputDebugString("Creating file: " + filename);
        file = new std::ofstream(filepath, std::ios::binary);
        if (!file->is_open()) {
            std::cerr << "Erreur : Impossible d'ouvrir le fichier." << std::endl;
            return false;
        }

        int error;
        encoder = opus_encoder_create(sampleRate, numChannels, OPUS_APPLICATION_AUDIO, &error);
        if (error != OPUS_OK) {
            std::cerr << "Erreur : Impossible de créer l'encodeur Opus." << std::endl;
            delete file;
            file = nullptr;
            return false;
        }

        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
        return true;
    }

    void write(const std::vector<int16_t>& samples) {
        if (!file || !file->is_open() || !encoder) return;
        juce::Logger::outputDebugString("Writing in opus file");

        unsigned char output[4000];
        int numBytes = opus_encode(encoder, samples.data(), samples.size(), output, sizeof(output));
        if (numBytes > 0) {
            file->write(reinterpret_cast<const char*>(output), numBytes);
        }
    }

    void close() {
        if (file && file->is_open()) {
            file->close();
            delete file;
            file = nullptr;
        }
        if (encoder) {
            opus_encoder_destroy(encoder);
            encoder = nullptr;
        }
    }

private:
    uint32_t sampleRate;
    int bitrate;
    uint16_t numChannels;
    std::ofstream* file;
    OpusEncoder* encoder;
};