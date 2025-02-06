#pragma once
#include "AudioAppPlayer.h"

AudioAppPlayer::AudioAppPlayer()
{
    setAudioChannels(0, 2); // Pas d'entrée, sortie stéréo
    EventManager::getInstance().addListener(this);
}

AudioAppPlayer::~AudioAppPlayer() {
    EventManager::getInstance().removeListener(this);
}

void AudioAppPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    juce::ignoreUnused(samplesPerBlockExpected);
    currentSampleRate = sampleRate;
    currentSampleIndex = 0;
}


void AudioAppPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) {
    if (bufferToFill.buffer == nullptr) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    auto* leftChannel = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightChannel = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    int samplesToCopy = std::min(audioBlock.size() / 2, static_cast<size_t>(bufferToFill.numSamples));

    for (int sample = 0; sample < samplesToCopy; ++sample) {
        leftChannel[sample] = audioBlock[sample];
        rightChannel[sample] = audioBlock[sample];
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

void AudioAppPlayer::onAudioBlockSent(const AudioBlockSentEvent &event) {
    audioBlock = event.data;
}
