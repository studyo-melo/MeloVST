#pragma once
#include <juce_core/juce_core.h>

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
}