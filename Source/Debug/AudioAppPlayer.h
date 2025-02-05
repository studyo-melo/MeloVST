#pragma once
#include "AudioAppPlayer.h"
#include "../Utils/FileUtils.h"
#include "../Utils/VectorUtils.h"
#include <thread>
#include <atomic>
#include <vector>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>

#include "../Encoders/OpusCodecWrapper.h"
#include "../Events/EventListener.h"
#include "../Events/EventManager.h"
#include "../Files/OpusFileHandler.h"
#include "../Files/WavFileHandler.h"

// Ces constantes sont celles que vous utilisez déjà
#define SAMPLE_RATE 48000
#define BITRATE 64000
#define NUM_CHANNELS 2
#define BIT_DEPTH 16
#define OPUS_FRAME_SIZE 20    // en millisecondes
#define OPUS_SAMPLE_RATE 44100


template <typename T>
class CircularBuffer
{
public:
    // Constructeur : la capacité est fixée lors de l'instanciation
    explicit CircularBuffer(int capacity)
        : buffer(capacity), capacity(capacity), readIndex(0), writeIndex(0), availableSamples(0)
    {
        if (capacity <= 0)
            throw std::invalid_argument("La capacité doit être supérieure à 0");
    }

    // Ajoute des échantillons dans le tampon
    // Renvoie le nombre d'échantillons effectivement ajoutés (si le tampon est plein, certains échantillons ne seront pas ajoutés)
    int pushSamples(const T* data, int numSamples)
    {
        int samplesPushed = 0;
        for (int i = 0; i < numSamples; ++i)
        {
            if (availableSamples < capacity)
            {
                buffer[writeIndex] = data[i];
                writeIndex = (writeIndex + 1) % capacity;
                ++availableSamples;
                ++samplesPushed;
            }
            else
            {
                // Le tampon est plein, on arrête l'insertion
                break;
            }
        }
        return samplesPushed;
    }

    // Retire des échantillons du tampon et les copie dans destination.
    // Renvoie le nombre d'échantillons effectivement lus.
    int popSamples(T* destination, int numSamples)
    {
        int samplesPopped = 0;
        for (int i = 0; i < numSamples; ++i)
        {
            if (availableSamples > 0)
            {
                destination[i] = buffer[readIndex];
                readIndex = (readIndex + 1) % capacity;
                --availableSamples;
                ++samplesPopped;
            }
            else
            {
                // Plus d'échantillons disponibles
                break;
            }
        }
        return samplesPopped;
    }

    // Renvoie le nombre d'échantillons actuellement disponibles dans le tampon
    int getNumAvailableSamples() const { return availableSamples; }

    // Renvoie l'espace libre (nombre d'échantillons pouvant être ajoutés)
    int getFreeSpace() const { return capacity - availableSamples; }

    // Vide le tampon
    void clear()
    {
        readIndex = 0;
        writeIndex = 0;
        availableSamples = 0;
    }

private:
    std::vector<T> buffer;
    int capacity;
    int readIndex;
    int writeIndex;
    int availableSamples;
};

class AudioAppPlayer : public juce::AudioAppComponent, public EventListener {
public:
    AudioAppPlayer();
    ~AudioAppPlayer() override;

    void createFiles();
    void finalizeFiles();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;
    void releaseResources() override;

    // Callback custom lorsque le bloc audio est prêt à être traité (issu d'un EventManager, par exemple)
    void onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) override;

private:
    // Méthode exécutée par le thread d'encodage
    void processingThreadFunction();

    // Variables pour gérer le thread d'encodage
    std::atomic<bool> threadRunning { true };
    std::thread encodingThread;

    // Tampon circulaire pour stocker les données audio (interleaved)
    CircularBuffer<float> circularBuffer;  // implémentation à fournir ou basée sur juce::AbstractFifo
    juce::CriticalSection circularBufferLock;

    // Utilitaires de fichiers
    WavFileHandler vanillaWavFile;
    WavFileHandler decodedWavFileHandler;
    OpusFileHandler encodedOpusFileHandler;

    // Vos objets d'encodage/décodage
    OpusCodecWrapper opusCodec;

    // Buffer temporaire reçu depuis l'event
    std::vector<float> audioBlock;
    int currentNumSamples = 0;
    int currentSampleIndex = 0;

    double currentSampleRate = SAMPLE_RATE;
};
