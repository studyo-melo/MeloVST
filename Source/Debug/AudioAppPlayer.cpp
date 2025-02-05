#pragma once
#include "AudioAppPlayer.h"

#include "../Utils/FileUtils.h"
#include "../Utils/VectorUtils.h"
#include <stdio.h>
#include <vector>

AudioAppPlayer::AudioAppPlayer()
    : circularBuffer(SAMPLE_RATE * NUM_CHANNELS),
      vanillaWavFile(SAMPLE_RATE, BIT_DEPTH, NUM_CHANNELS),
      decodedWavFileHandler(SAMPLE_RATE, BIT_DEPTH, NUM_CHANNELS),
      encodedOpusFileHandler(OPUS_SAMPLE_RATE, BITRATE, NUM_CHANNELS),
      opusCodec(OPUS_SAMPLE_RATE, SAMPLE_RATE, NUM_CHANNELS, OPUS_FRAME_SIZE)
// resampler(OPUS_SAMPLE_RATE, SAMPLE_RATE, NUM_CHANNELS)
{
    setAudioChannels(0, 2); // Pas d'entrée, sortie stéréo
    EventManager::getInstance().addListener(this);
    createFiles();

    // Démarrer le thread d'encodage
    encodingThread = std::thread(&AudioAppPlayer::processingThreadFunction, this);
}

AudioAppPlayer::~AudioAppPlayer() {
    // Indiquer l'arrêt du thread et attendre sa fin
    threadRunning = false;
    if (encodingThread.joinable())
        encodingThread.join();

    shutdownAudio();
    EventManager::getInstance().removeListener(this);
    finalizeFiles();
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
    if (bufferToFill.buffer == nullptr) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    // On vérifie que le buffer audio (audioBlock) contient des données
    if (!audioBlock.empty()) {
        const int numSamples = static_cast<int>(audioBlock.size()); {
            juce::ScopedLock sl(circularBufferLock);
            circularBuffer.pushSamples(audioBlock.data(), numSamples);
        }
        // On vide le buffer temporaire pour éviter une réécriture multiple
        audioBlock.clear();
    }

    // Le callback n'envoie pas de son vers la sortie
    bufferToFill.clearActiveBufferRegion();
}

void AudioAppPlayer::processingThreadFunction() {
    // Calcul du nombre d'échantillons par canal pour une trame de 20 ms
    const int frameSamples = static_cast<int>(SAMPLE_RATE * OPUS_FRAME_SIZE / 1000.0); // 48000 * 20/1000 = 960
    const int totalFrameSamples = frameSamples * NUM_CHANNELS; // Pour un signal interleaved

    while (threadRunning) {
        bool frameAvailable = false;
        std::vector<float> frameData(totalFrameSamples); {
            // Protéger l'accès au tampon circulaire
            juce::ScopedLock sl(circularBufferLock);
            if (circularBuffer.getNumAvailableSamples() >= totalFrameSamples) {
                circularBuffer.popSamples(frameData.data(), totalFrameSamples);
                frameAvailable = true;
            }
        }

        if (frameAvailable) {
            // (1) Écriture du signal d'origine dans le fichier WAV
            vanillaWavFile.write(frameData, totalFrameSamples);

            // (2) Encodage Opus (la fonction encode attend le nombre d'échantillons par canal)
            std::vector<unsigned char> opusPacket = opusCodec.encode_float(frameData, frameSamples);

            if (!opusPacket.empty()) {
                // (3) Écriture du paquet Opus dans le fichier .opus
                encodedOpusFileHandler.write(opusPacket);

                // (4) Optionnel : décodage pour vérification
                std::vector<float> decodedFrame = opusCodec.decode_float(opusPacket);
                if (!decodedFrame.empty())
                    decodedWavFileHandler.write(decodedFrame, decodedFrame.size());
            } else {
                // Gestion d'erreur d'encodage : éventuellement journaliser ou notifier
            }
        } else {
            // Si pas assez d'échantillons, on attend un peu
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

void AudioAppPlayer::releaseResources() {
    audioBlock.clear();
}

void AudioAppPlayer::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    audioBlock = event.data;
    currentNumSamples = event.numSamples;
    currentSampleIndex = 0;
}
