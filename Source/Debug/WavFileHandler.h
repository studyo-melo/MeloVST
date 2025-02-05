#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

class WavFileHandler {
public:
    WavFileHandler(int sampleRate, int bitsPerSample, int numChannels) : file(nullptr), sampleRate(sampleRate), bitsPerSample(bitsPerSample), numChannels(numChannels), dataSize(0) {}
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
        writeWavHeader();
        return true;
    }

    void write(const std::vector<int16_t>& samples) {
        if (file && file->is_open()) {
            file->write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
            dataSize += samples.size() * sizeof(int16_t);
        }
    }

    void close() {
        if (file && file->is_open()) {
            juce::Logger::outputDebugString("Closing file");
            finalizeWavHeader();
            file->close();
            delete file;
            file = nullptr;
        }
    }

private:
    std::ofstream* file;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
    uint16_t numChannels;
    uint32_t dataSize;

    void writeWavHeader() {
        char header[44] = {0};
        std::memcpy(header, "RIFF", 4);
        std::memcpy(header + 8, "WAVEfmt ", 8);
        uint32_t subChunk1Size = 16;
        uint16_t audioFormat = 1;
        uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
        uint16_t blockAlign = numChannels * bitsPerSample / 8;

        std::memcpy(header + 4, &dataSize, 4);
        std::memcpy(header + 16, &subChunk1Size, 4);
        std::memcpy(header + 20, &audioFormat, 2);
        std::memcpy(header + 22, &numChannels, 2);
        std::memcpy(header + 24, &sampleRate, 4);
        std::memcpy(header + 28, &byteRate, 4);
        std::memcpy(header + 32, &blockAlign, 2);
        std::memcpy(header + 34, &bitsPerSample, 2);
        std::memcpy(header + 36, "data", 4);
        std::memcpy(header + 40, &dataSize, 4);

        file->write(header, 44);
    }

    void finalizeWavHeader() {
        file->seekp(4, std::ios::beg);
        uint32_t fileSize = 36 + dataSize;
        file->write(reinterpret_cast<const char*>(&fileSize), 4);
        file->seekp(40, std::ios::beg);
        file->write(reinterpret_cast<const char*>(&dataSize), 4);
    }
};