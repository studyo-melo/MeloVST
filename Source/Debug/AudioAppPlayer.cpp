#pragma once
#include "AudioAppPlayer.h"

#include "../Utils/FileUtils.h"


AudioAppPlayer::AudioAppPlayer(): opusCodec(20, 2, 48000) {
    setAudioChannels(0, 2); // Pas d'entrée, sortie stéréo
    EventManager::getInstance().addListener(this);

    wavFile = FileUtils::initializeWavFile("1_base_audio.wav");
    opusFile = FileUtils::initializeOpusFile("1_encoded_opus_audio.opus");
    decodedWavFile = FileUtils::initializeWavFile("1_decoded_opus_audio.wav");
}

AudioAppPlayer::~AudioAppPlayer() {
    shutdownAudio();
    EventManager::getInstance().removeListener(this);
}

void AudioAppPlayer::createFiles() {
    wavFile = FileUtils::initializeWavFile("1_base_audio.wav");
    opusFile = FileUtils::initializeOpusFile("1_encoded_opus_audio.opus");
    decodedWavFile = FileUtils::initializeWavFile("1_decoded_opus_audio.wav");
}

void AudioAppPlayer::finalizeFiles() {
    FileUtils::finalizeWavFile(wavFile);
    FileUtils::finalizeOpusFile(opusFile, opusData.size());
    FileUtils::finalizeWavFile(decodedWavFile);
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

    // Si audioBlock est vide, ne pas traiter
    // if (audioBlock.empty()) {
    //     bufferToFill.clearActiveBufferRegion();
    //     return;
    // }

    FileUtils::appendWavData(wavFile, audioBlock);

    std::vector<int8_t> opusEncodedAudioBlock = opusCodec.encode(audioBlock);
    if (opusEncodedAudioBlock.empty()) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    opusData.resize(opusData.size() + opusEncodedAudioBlock.size());
    opusData.insert(opusData.end(), opusEncodedAudioBlock.begin(), opusEncodedAudioBlock.end());
    FileUtils::appendOpusData(opusFile, opusEncodedAudioBlock);

    std::vector<int16_t> decodedAudioBlock = opusCodec.decode(opusEncodedAudioBlock);
    if (decodedAudioBlock.empty()) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    FileUtils::appendWavData(decodedWavFile, decodedAudioBlock);

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
