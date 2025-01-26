#pragma once
#include "AudioAppPlayer.h"


AudioAppPlayer::AudioAppPlayer() {
    setAudioChannels(0, 2); // Pas d'entrée, sortie stéréo
    EventManager::getInstance().addListener(this);
}

AudioAppPlayer::~AudioAppPlayer() {
    shutdownAudio();
    EventManager::getInstance().removeListener(this);
}

void AudioAppPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    juce::ignoreUnused(samplesPerBlockExpected);
    currentSampleRate = sampleRate;
    currentSampleIndex = 0;
}


// std::vector<uint8_t> opusEncodedAudioBlock; // Déclaration de la variable à l'extérieur
// opusCodec.encode(audioBlock, [&opusEncodedAudioBlock](std::vector<uint8_t>&& encodedData) {
//     opusEncodedAudioBlock = std::move(encodedData); // Capturer par référence
// });
// if (opusEncodedAudioBlock.empty()) {
//     bufferToFill.clearActiveBufferRegion();
//     return;
// }
//
// std::vector<int16_t> decodedAudioBlock = opusCodec.decode(opusEncodedAudioBlock);
// if (decodedAudioBlock.empty()) {
//     bufferToFill.clearActiveBufferRegion();
//     return;
// }
void AudioAppPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    if (audioBlock.empty() || !bufferToFill.buffer) {
        bufferToFill.clearActiveBufferRegion(); // Efface la région active si aucune donnée
        return;
    }

    const int numChannels = bufferToFill.buffer->getNumChannels();
    const int numSamplesToCopy = std::min(static_cast<int>(audioBlock.size()), bufferToFill.numSamples);

    // Remplir les canaux gauche et droit
    for (int channel = 0; channel < numChannels; ++channel) {
        auto* channelData = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

        // Copie des données audio normalisées
        for (int i = 0; i < numSamplesToCopy; ++i) {
            channelData[i] = static_cast<float>(audioBlock[i]) / 32768.0f; // Normalisation entre -1.0 et 1.0
        }

        // Complète le reste avec des zéros si nécessaire
        for (int i = numSamplesToCopy; i < bufferToFill.numSamples; ++i) {
            channelData[i] = 0.0f;
        }
    }
}



void AudioAppPlayer::releaseResources() {
    audioBlock.clear();
}

void AudioAppPlayer::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent& event) {
    audioBlock.resize(event.data.size());
    audioBlock = event.data;

    currentNumSamples = event.numSamples;
    currentSampleIndex = 0;
}
