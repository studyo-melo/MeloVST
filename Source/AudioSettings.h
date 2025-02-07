#pragma once

class AudioSettings
{
public:
    static AudioSettings& getInstance() {
        static AudioSettings instance;
        return instance;
    }

    [[nodiscard]] int getSampleRate() const noexcept {
        return sampleRate;
    }

    [[nodiscard]] int getNumChannels() const noexcept {
        return numChannels;
    }

    [[nodiscard]] int getBlockSize() const noexcept {
        return blockSize;
    }

    [[nodiscard]] int getBitDepth() const noexcept {
        return bitDepth;
    }

    [[nodiscard]] int getOpusSampleRate() const noexcept {
        return opusSampleRate;
    }

    // Mutateurs pour définir les paramètres audio
    void setSampleRate(int newSampleRate) noexcept {
        sampleRate = newSampleRate;
    }

    void setOpusSampleRate(int newOpusSampleRate) noexcept {
        opusSampleRate = newOpusSampleRate;
    }

    void setBlockSize(int newBlockSize) noexcept {
        blockSize = newBlockSize;
    }

    void setNumChannels(int newNumChannels) noexcept {
        numChannels = newNumChannels;
    }

    void setBitDepth(int newBitDepth) noexcept {
        bitDepth = newBitDepth;
    }


private:
    // Constructeur et destructeur privés pour le Singleton
    AudioSettings() = default;
    ~AudioSettings() = default;

    // Suppression des méthodes de copie et d'affectation pour garantir l'unicité
    AudioSettings(const AudioSettings&) = delete;
    AudioSettings& operator=(const AudioSettings&) = delete;

    // Données membres pour stocker les paramètres audio
    int sampleRate = 0;
    int blockSize = 0;
    int numChannels = 0;
    int bitDepth = 0;
    int opusSampleRate = 0;
};
