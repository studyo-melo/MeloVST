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
        bufferToFill.clearActiveBufferRegion(); // Efface la région active si aucune donnée
        return;
    }

    // if (std::all_of(audioBlock.begin(), audioBlock.end(), [](const int16_t sample) { return sample == 0; })){
    //     bufferToFill.clearActiveBufferRegion(); // Efface le buffer si toutes les données sont nulles
    //     return;
    // }

    std::vector<int8_t> opusEncodedAudioBlock = opusCodec.encode_in_place(audioBlock);
    if (opusEncodedAudioBlock.empty()) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    std::vector<int16_t> decodedAudioBlock = opusCodec.decode(opusEncodedAudioBlock);
    if (decodedAudioBlock.empty()) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    auto* leftChannel = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightChannel = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    int samplesToCopy = std::min(decodedAudioBlock.size() / 2, static_cast<size_t>(bufferToFill.numSamples));

    for (int sample = 0; sample < samplesToCopy; ++sample) {
        float normalizedSample = static_cast<float>(decodedAudioBlock[sample]) / 32768.0f; // Normaliser PCM 16 bits
        leftChannel[sample] = normalizedSample;
        rightChannel[sample] = normalizedSample; // Mono -> Stéréo
    }

    // Remplir le reste avec des zéros si besoin
    for (int sample = samplesToCopy; sample < bufferToFill.numSamples; ++sample) {
        leftChannel[sample] = 0.0f;
        rightChannel[sample] = 0.0f;
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
