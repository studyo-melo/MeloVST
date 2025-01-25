#pragma once
#include "AudioAppPlayer.h"


AudioAppPlayer::AudioAppPlayer() : opusCodec() {
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

    auto* leftChannel = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightChannel = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    // Encode puis décode le bloc audio
    auto encodedAudioBlock = opusCodec.encode(audioBlock.data());
    auto decodedAudioBlock = opusCodec.decode(encodedAudioBlock);

    for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
        if (currentSampleIndex >= audioBlock.size())
            currentSampleIndex = 0; // Boucle sur le bloc audio

        float currentSample = audioBlock[currentSampleIndex++];
        leftChannel[sample] = currentSample;
        rightChannel[sample] = currentSample;
    }
}

void AudioAppPlayer::releaseResources() {
    audioBlock.clear();
}

void AudioAppPlayer::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent& event) {
    audioBlock.resize(event.data.size());
    std::copy(event.data.begin(), event.data.end(), audioBlock.begin());

    currentNumSamples = event.numSamples;
    currentSampleIndex = 0;
}
