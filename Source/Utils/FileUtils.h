#pragma once
#include <juce_core/juce_core.h>
#include <iostream>
#include <fstream>
#include <fstream>
#include <vector>

namespace FileUtils {
    static std::string generateTimestampedFilename(const std::string& baseName, const std::string& extension) {
    // Obtenir l'heure actuelle
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    auto localTime = *std::localtime(&timeT);

    // Formater l'horodatage
    std::ostringstream oss;
    oss << baseName << "_"
        << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S") // Format : Année-Mois-Jour_Heure-Minute-Seconde
        << "." << extension;

    return oss.str();
    }

    inline std::string getFilePath(const std::string& filename) {
        const char* homeDir = getenv("HOME");
        if (!homeDir) {
            throw std::runtime_error("Impossible de localiser le répertoire personnel (HOME).");
        }

        std::string desktopPath = std::string(homeDir) + "/Desktop";
        return desktopPath + "/" + filename;
    }

    inline std::fstream initializeWavFile(const std::string& filename) {
        std::string filepath = getFilePath(filename);

        std::fstream wavFile(filepath, std::ios::binary | std::ios::out);
        if (!wavFile) {
            throw std::runtime_error("Failed to create WAV file.");
        }
        int sampleRate = 48000;//AudioSettings::getInstance().getSampleRate();
        int channels = 2; //AudioSettings::getInstance().getNumChannels();
        int blockSize = 1024; //AudioSettings::getInstance().getBlockSize();

        juce::Logger::outputDebugString("Initializing WAV file with " + std::to_string(sampleRate) + "Hz, " + std::to_string(channels) + " channels" + " and " + std::to_string(blockSize) + " block size");
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

    inline void appendWavData(std::fstream& wavFile, const std::vector<float>& pcmData) {
        for (float sample : pcmData) {
            if (sample < -1.0f || sample > 1.0f) {
                std::cerr << "Sample out of range: " << sample << std::endl;
            }
            int16_t intSample = static_cast<int16_t>(std::clamp(sample * 32767.0f, -32768.0f, 32767.0f));
            wavFile.write(reinterpret_cast<const char*>(&intSample), sizeof(int16_t));
        }
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

    inline void saveOpusToFile(const std::string& filename, const std::vector<int8_t>& opusData) {
       std::ofstream outFile(filename, std::ios::binary);
        if (!outFile) {
            throw std::runtime_error("Failed to open file for writing: " + filename);
        }

        // Écrire l'en-tête Ogg
        outFile.write("OggS", 4); // Identifiant Ogg
        outFile.put(0);            // Version
        outFile.put(0);            // Type de paquet (0 pour les données)
        outFile.put(0);            // Granule position (non utilisé ici)
        outFile.put(0);            // Granule position (non utilisé ici)
        outFile.put(0);            // Granule position (non utilisé ici)
        outFile.put(0);            // Granule position (non utilisé ici)
        outFile.put(0);            // Bitstream serial number (à générer, ici 0)
        outFile.put(0);            // Bitstream serial number (à générer, ici 0)
        outFile.put(0);            // Index de page (à générer)
        outFile.put(0);            // Index de page (à générer)

        // Calculer la taille de l'en-tête
        std::streampos opusHeaderStart = outFile.tellp();

        // Écrire les données Opus (encodées)
        outFile.write(reinterpret_cast<const char*>(opusData.data()), opusData.size());

        // Écrire le reste des informations Ogg
        std::streampos opusDataStart = outFile.tellp();
        std::streamsize opusDataSize = opusData.size();

        // Retourner à l'en-tête pour mettre à jour les informations
        outFile.seekp(0, std::ios::beg);

        // Mettre à jour la taille de l'en-tête et d'autres informations ici
        // (génération de la taille et d'autres informations si nécessaire)

        // Écrire à nouveau l'en-tête Ogg
        outFile.write("OggS", 4);
        outFile.put(0); // Version
        outFile.put(0); // Type de paquet
        // Granule position - le nombre d'échantillons dans le flux, cela peut être ajusté si nécessaire
        outFile.put(static_cast<char>((opusDataSize >> 24) & 0xFF)); // Granule position
        outFile.put(static_cast<char>((opusDataSize >> 16) & 0xFF)); // Granule position
        outFile.put(static_cast<char>((opusDataSize >> 8) & 0xFF));  // Granule position
        outFile.put(static_cast<char>(opusDataSize & 0xFF)); // Granule position
        outFile.put(0); // Bitstream serial number (0)
        outFile.put(0); // Index de page (0)
        outFile.put(0); // Index de page (0)

        // Écrire les données Opus
        outFile.seekp(opusDataStart, std::ios::beg);
        outFile.write(reinterpret_cast<const char*>(opusData.data()), opusData.size());

        outFile.close();
    }

    inline void writeWav(const std::string& filename, const std::vector<int16_t>& pcmData, int sampleRate, int channels) {
        std::ofstream outFile(getFilePath(filename), std::ios::binary);
        if (!outFile) {
            throw std::runtime_error("Failed to open file for writing: " + filename);
        }

        // WAV header
        uint32_t dataSize = pcmData.size() * sizeof(int16_t);
        uint32_t fileSize = 36 + dataSize; // 36 = header size - 8 bytes
        outFile.write("RIFF", 4);
        outFile.write(reinterpret_cast<const char*>(&fileSize), 4);
        outFile.write("WAVE", 4);
        outFile.write("fmt ", 4);

        uint32_t fmtChunkSize = 16;
        uint16_t audioFormat = 1; // PCM
        uint16_t numChannels = static_cast<uint16_t>(channels);
        uint32_t byteRate = sampleRate * channels * sizeof(int16_t);
        uint16_t blockAlign = channels * sizeof(int16_t);
        uint16_t bitsPerSample = 16;

        outFile.write(reinterpret_cast<const char*>(&fmtChunkSize), 4);
        outFile.write(reinterpret_cast<const char*>(&audioFormat), 2);
        outFile.write(reinterpret_cast<const char*>(&numChannels), 2);
        outFile.write(reinterpret_cast<const char*>(&sampleRate), 4);
        outFile.write(reinterpret_cast<const char*>(&byteRate), 4);
        outFile.write(reinterpret_cast<const char*>(&blockAlign), 2);
        outFile.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

        outFile.write("data", 4);
        outFile.write(reinterpret_cast<const char*>(&dataSize), 4);

        // PCM data
        outFile.write(reinterpret_cast<const char*>(pcmData.data()), dataSize);

        outFile.close();
    }
}