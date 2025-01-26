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

void AudioAppPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    if (audioBlock.empty() || !bufferToFill.buffer) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    // Convertit le bloc audio en int16_t
    std::vector<int16_t> audioBlockInt16(audioBlock.size());
    for (size_t i = 0; i < audioBlock.size(); ++i) {
        audioBlockInt16[i] = static_cast<int16_t>(audioBlock[i] * 32767.0f);
    }

    if (std::all_of(audioBlockInt16.begin(), audioBlockInt16.end(), [](const int16_t sample) { return sample == 0; })
    ){
        bufferToFill.clearActiveBufferRegion(); // Efface le buffer si toutes les données sont nulles
        return;
    }

    std::vector<uint8_t> opusEncodedAudioBlock; // Déclaration de la variable à l'extérieur
    opusCodec.encode(std::move(audioBlockInt16), [&opusEncodedAudioBlock](std::vector<uint8_t>&& encodedData) {
        opusEncodedAudioBlock = std::move(encodedData); // Capturer par référence
    });
    // Vérifie que les données décodées ont la bonne taille
    if (opusEncodedAudioBlock.empty()) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    // Décode le bloc audio
    std::vector<int16_t> decodedAudioBlock = opusCodec.decode(opusEncodedAudioBlock);
    //
    // // // Vérifie que les données décodées ont la bonne taille
    if (decodedAudioBlock.empty()) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    auto* leftChannel = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightChannel = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    // Copie les 520 premiers échantillons décodés dans le tampon audio de JUCE
    const int samplesToCopy = std::min(static_cast<int>(decodedAudioBlock.size()), 520); // Limite à 520 échantillons
    for (int i = 0; i < samplesToCopy; ++i) {
        // Remplir les deux canaux avec les échantillons décodés
        leftChannel[i] = static_cast<float>(decodedAudioBlock[i]) / 32768.0f; // Normaliser entre -1.0 et 1.0
        rightChannel[i] = leftChannel[i]; // Si vous souhaitez un signal mono pour les deux canaux
    }

    // Si le nombre d'échantillons est inférieur à 520, remplissez le reste du tampon avec des zéros
    for (int i = samplesToCopy; i < bufferToFill.numSamples; ++i) {
        leftChannel[i] = 0.0f;
        rightChannel[i] = 0.0f;
    }
}


void AudioAppPlayer::releaseResources() {
    audioBlock.clear();
}

void AudioAppPlayer::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent& event) {
    audioBlock.resize(event.data.size());
    audioBlock = event.data;
    // std::copy(event.data.begin(), event.data.end(), audioBlock.begin());

    currentNumSamples = event.numSamples;
    currentSampleIndex = 0;
}
