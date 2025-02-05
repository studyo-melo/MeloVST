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

    [[nodiscard]] int getBitDepth() const noexcept {
        return bitDepth;
    }

    // Mutateurs pour définir les paramètres audio
    void setSampleRate(int newSampleRate) noexcept {
        sampleRate = newSampleRate;
    }

    void setNumChannels(int newNumChannels) noexcept {
        numChannels = newNumChannels;
    }

    void setBitDepth(int newBitDepth) noexcept {
        bitDepth = newBitDepth;
    }

    void setSamplePerBlock(int newSamplePerBlock) noexcept {
        samplePerBlock = newSamplePerBlock;
    }

    [[nodiscard]] int getSamplePerBlock() const noexcept {
        return samplePerBlock;
    }

    void setBitrate(int newBitrate) noexcept {
        bitrate = newBitrate;
    }

    [[nodiscard]] int getBitrate() const noexcept {
        return bitrate;
    }

    void setOpusSampleRate(int newOpusSampleRate) noexcept {
        opusSampleRate = newOpusSampleRate;
    }

    [[nodiscard]] int getOpusSampleRate() const noexcept {
        return opusSampleRate;
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
    int samplePerBlock = 0;
    int numChannels = 0;
    int bitrate = 0;
    int opusSampleRate = 0;
    int bitDepth = 0;
};
