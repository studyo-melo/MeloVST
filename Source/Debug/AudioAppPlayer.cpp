#pragma once
#include "AudioAppPlayer.h"


AudioAppPlayer::AudioAppPlayer() : opusCodec(), opusEncoder(48000, 2, 20), opusDecoder(48000, 2, 20) {
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
    opusEncoder.Encode(std::move(audioBlockInt16), [&opusEncodedAudioBlock](std::vector<uint8_t>&& encodedData) {
        opusEncodedAudioBlock = std::move(encodedData); // Capturer par référence
    });
    // Vérifie que les données décodées ont la bonne taille
    if (opusEncodedAudioBlock.empty()) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    // Décode le bloc audio
    std::vector<int16_t> decodedAudioBlock = opusDecoder.Decode(opusEncodedAudioBlock);
    //
    // // // Vérifie que les données décodées ont la bonne taille
    if (decodedAudioBlock.empty()) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    // Remplit les canaux audio avec les données décodées
    for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
        // if (currentSampleIndex >= opusEncodedAudioBlock.size())
        //     currentSampleIndex = 0; // Boucle sur le bloc audio

        float currentSample = decodedAudioBlock[currentSampleIndex++] / 32767.0f;
        leftChannel[sample] = currentSample;
        if (bufferToFill.buffer->getNumChannels() > 1){
            rightChannel[sample] = currentSample;
        }
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
