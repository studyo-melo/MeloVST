#pragma once
#include "AudioAppPlayer.h"

#include "../Utils/FileUtils.h"
#include "../Utils/VectorUtils.h"

#define SAMPLE_RATE 48000
#define BITRATE 64000
#define NUM_CHANNELS 2
#define BIT_DEPTH 16
#define OPUS_FRAME_SIZE 20
#define OPUS_SAMPLE_RATE 44100

AudioAppPlayer::AudioAppPlayer(): vanillaWavFile(SAMPLE_RATE, BIT_DEPTH, NUM_CHANNELS),
                                  decodedWavFileHandler(SAMPLE_RATE, BIT_DEPTH, NUM_CHANNELS),
                                  encodedOpusFileHandler(SAMPLE_RATE, BITRATE, NUM_CHANNELS),
                                  opus_encoder_juce_(SAMPLE_RATE, NUM_CHANNELS, BITRATE),
                                  opusCodec(SAMPLE_RATE, NUM_CHANNELS, OPUS_FRAME_SIZE),
                                  resampler(OPUS_SAMPLE_RATE, SAMPLE_RATE, NUM_CHANNELS) {
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
    encodedOpusFileHandler.create("2_encoded_opus_audio.opus");
    decodedWavFileHandler.create("1_decoded_opus_audio.wav");
}

void AudioAppPlayer::finalizeFiles() {
    vanillaWavFile.close();
    encodedOpusFileHandler.close();
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

    // std::vector<float> resampledDoubleAudioBlock = resampler.resampleFromFloat(audioBlock);

    std::vector<int16_t> audioBlockInt16 = VectorUtils::convertFloatToInt16(audioBlock.data(), audioBlock.size());
    vanillaWavFile.write(audioBlockInt16);
    // auto encodedData = opus_encoder_juce_.processAudioBlock(audioBlock);
    std::vector<unsigned char> opusEncodedAudioBlock;
    opusCodec.encode(audioBlockInt16, [this, &opusEncodedAudioBlock](std::vector<unsigned char> opus) {
        opusEncodedAudioBlock = opus;
    });
    // std::vector<unsigned char> opusEncodedAudioBlock = opusCodec.encode_float(audioBlock);
    encodedOpusFileHandler.write(opusEncodedAudioBlock);
    // std::vector<int16_t> decodedAudioBlock(MAX_OPUS_PACKET_SIZE);
    // opusCodec.decode(opusEncodedAudioBlock, decodedAudioBlock);
    // vanillaWavFile.write(decodedAudioBlock);

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
