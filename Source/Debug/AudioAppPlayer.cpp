#pragma once
#include "AudioAppPlayer.h"

#include "../Utils/FileUtils.h"
#include "../Utils/VectorUtils.h"


AudioAppPlayer::AudioAppPlayer(): vanillaWavFile(48000, 16, 2),
                                  decodedWavFileHandler(48000, 16, 2),
                                  opusCodec(48000, 2, 20),
                                  resampler(44100, 48000, 2) {
    setAudioChannels(0, 2); // Pas d'entrée, sortie stéréo
    EventManager::getInstance().addListener(this);

    createFiles();
}

AudioAppPlayer::~AudioAppPlayer() {
    shutdownAudio();
    EventManager::getInstance().removeListener(this);
}

void AudioAppPlayer::createFiles() {
    vanillaWavFile.create("1_base_audio.wav");
    decodedWavFileHandler.create("1_decoded_opus_audio.wav");
}

void AudioAppPlayer::finalizeFiles() {
    vanillaWavFile.close();
    decodedWavFileHandler.close();
}

void AudioAppPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    juce::ignoreUnused(samplesPerBlockExpected);
    currentSampleRate = sampleRate;
    currentSampleIndex = 0;
}


void AudioAppPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) {
    if (audioBlock.empty() || !bufferToFill.buffer) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    std::vector<float> resampledDoubleAudioBlock = resampler.resampleFromFloat(audioBlock);

    std::vector<int16_t> audioBlockInt16 = VectorUtils::convertFloatToInt16(resampledDoubleAudioBlock.data(), resampledDoubleAudioBlock.size());
    vanillaWavFile.write(audioBlockInt16);
    // const std::vector<unsigned char> opusEncodedAudioBlock = opusCodec.encode_float(audioBlock);
    // opusData.resize(opusData.size() + opusEncodedAudioBlock.size());
    // opusData.insert(opusData.end(), opusEncodedAudioBlock.begin(), opusEncodedAudioBlock.end());
    // FileUtils::appendOpusData(opusFile, opusEncodedAudioBlock);
    // const std::vector<float> decodedAudioBlock = opusCodec.decode_float(opusEncodedAudioBlock);
    // //
    // std::vector<int16_t> decodedAudioBlock = opusCodec.decode(opusEncodedAudioBlock);
    // if (decodedAudioBlock.empty()) {
    //     bufferToFill.clearActiveBufferRegion();
    //     return;
    // }
    //
    // FileUtils::appendWavData(decodedWavFile, decodedAudioBlock);

    // auto* leftChannel = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    // auto* rightChannel = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
    //
    // int samplesToCopy = std::min(decodedAudioBlock.size() / 2, static_cast<size_t>(bufferToFill.numSamples));
    //
    // for (int sample = 0; sample < samplesToCopy; ++sample) {
    //     leftChannel[sample] = decodedAudioBlock[sample];
    //     rightChannel[sample] = decodedAudioBlock[sample];
    // }
    //
    // // Remplir le reste avec des zéros si besoin
    // for (int sample = samplesToCopy; sample < bufferToFill.numSamples; ++sample) {
    //     leftChannel[sample] = 0.0f;
    //     rightChannel[sample] = 0.0f;
    // }
}


void AudioAppPlayer::releaseResources() {
    audioBlock.clear();
}

void AudioAppPlayer::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    audioBlock = event.data;
    currentNumSamples = event.numSamples;
    currentSampleIndex = 0;
}
