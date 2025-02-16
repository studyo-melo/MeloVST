#pragma once
#include "DebugAudioAppPlayer.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>
#include "../Common/EventListener.h"
#include "../Common/EventManager.h"

// Comparateur pour la priorité (ordre croissant des timestamps)
struct CompareTimestamp {
    bool operator()(const AudioBlockReceivedDecodedEvent& a, const AudioBlockReceivedDecodedEvent& b) {
        return a.timestamp > b.timestamp; // Le plus ancien en haut de la file
    }
};
class DebugAudioAppPlayer final : public juce::AudioAppComponent, public EventListener
{
public:
    DebugAudioAppPlayer()
    {
        setAudioChannels (0, 2); // 0 entrées, 2 sorties
        EventManager::getInstance().addListener(this);
    }

    ~DebugAudioAppPlayer() override
    {
        shutdownAudio();
        EventManager::getInstance().removeListener(this);
    }

    // Préparation audio : on peut stocker le sampleRate courant
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        currentSampleRate = sampleRate;
    }

    // Ce callback est appelé par le thread audio de JUCE
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        if (bufferToFill.buffer == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        // Obtenir les pointeurs pour chaque canal (on considère ici le cas stereo)
        auto* leftChannel  = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* rightChannel = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
        int numSamples = bufferToFill.numSamples;
        int samplesWritten = 0;

        while (samplesWritten < numSamples)
        {
            // Si le bloc courant est vide, tenter d'en récupérer un depuis la file
            if (currentBlock.empty())
            {
                if (audioQueue.empty())
                {
                    // Aucun bloc disponible : remplir avec du silence et sortir
                    std::fill(leftChannel + samplesWritten, leftChannel + numSamples, 0.0f);
                    std::fill(rightChannel + samplesWritten, rightChannel + numSamples, 0.0f);
                    return;
                }
                else
                {
                    // Récupérer le bloc avec le plus ancien timestamp
                    currentBlock = std::move(audioQueue.top().data);
                    audioQueue.pop();
                    currentSampleIndex = 0;
                }
            }

            // Calculer le nombre d'échantillons restant dans le bloc courant
            int samplesRemainingInBlock = static_cast<int>(currentBlock.size()) - currentSampleIndex;
            int samplesToCopy = std::min(numSamples - samplesWritten, samplesRemainingInBlock);

            // Copier les échantillons du bloc courant dans le buffer de sortie
            for (int i = 0; i < samplesToCopy; ++i)
            {
                float sampleValue = currentBlock[currentSampleIndex + i];
                leftChannel[samplesWritten + i]  = sampleValue;
                rightChannel[samplesWritten + i] = sampleValue; // duplication sur les 2 canaux (mono)
            }

            samplesWritten += samplesToCopy;
            currentSampleIndex += samplesToCopy;

            // Si le bloc courant est entièrement consommé, le vider
            if (currentSampleIndex >= currentBlock.size())
            {
                currentBlock.clear();
                currentSampleIndex = 0;
            }
        }
    }

    void releaseResources() override
    {
        // Libérer les ressources audio si besoin
    }

    // Méthode appelée par le gestionnaire d'événements lorsque qu'un bloc décodé est disponible.
    // Cette fonction peut être appelée depuis un thread différent (par exemple, le thread réseau).
    void onAudioBlockReceivedDecoded(const AudioBlockReceivedDecodedEvent &event)
    {
        // On ajoute le bloc décodé dans la file de priorité (triée par timestamp)
        // Cette opération devrait être thread-safe si appelée depuis un autre thread.
        // Pour simplifier, ici nous utilisons une protection simple.
        {
            const juce::ScopedLock sl(lock);
            audioQueue.push(event);
        }
    }

private:
    // File de priorité pour les blocs audio triés par timestamp
    std::priority_queue<AudioBlockReceivedDecodedEvent,
                        std::vector<AudioBlockReceivedDecodedEvent>,
                        CompareTimestamp> audioQueue;

    std::vector<float> currentBlock;  // Bloc audio actuellement en cours de lecture
    size_t currentSampleIndex = 0;      // Indice dans le bloc courant

    double currentSampleRate = 0;

    juce::CriticalSection lock;  // Protection simple pour la file de priorité si nécessaire
};