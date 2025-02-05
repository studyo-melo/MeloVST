#include "WebRTCAudioService.h"

#include "../Debug/DebugRTPWrapper.h"
#include "../Utils/AudioSettings.h"
#include "../Utils/AudioUtils.h"

WebRTCAudioService::WebRTCAudioService(): circularBuffer(SAMPLE_RATE * NUM_CHANNELS),
                                          vanillaWavFile(SAMPLE_RATE, BIT_DEPTH, NUM_CHANNELS),
                                          decodedWavFileHandler(SAMPLE_RATE, BIT_DEPTH, NUM_CHANNELS),
                                          encodedOpusFileHandler(SAMPLE_RATE, BITRATE, NUM_CHANNELS),
                                          opusCodec(48000, 2, 20),
                                          resampler(44100, 48000, 2) {
}

WebRTCAudioService::~WebRTCAudioService() {
    stopAudioThread();
}

void WebRTCAudioService::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    std::vector<float> audioBlock = event.data;
    currentNumSamples = event.numSamples;
    currentSampleIndex = 0;
    // On vérifie que le buffer audio (audioBlock) contient des données
    if (!audioBlock.empty()) {
        const int numSamples = static_cast<int>(audioBlock.size()); {
            juce::ScopedLock sl(circularBufferLock);
            circularBuffer.pushSamples(audioBlock.data(), numSamples);
        }
        // On vide le buffer temporaire pour éviter une réécriture multiple
        audioBlock.clear();
    }
}

void WebRTCAudioService::processingThreadFunction() {
    // Calcul du nombre d'échantillons par canal pour une trame de 20 ms
    const int frameSamples = static_cast<int>(SAMPLE_RATE * OPUS_FRAME_SIZE / 1000.0); // 48000 * 20/1000 = 960
    const int totalFrameSamples = frameSamples * NUM_CHANNELS; // Pour un signal interleaved
    while (threadRunning) {
        juce::Logger::outputDebugString("Running Thread Audio");
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

            // // (2) Encodage Opus (la fonction encode attend le nombre d'échantillons par canal)
            // std::vector<unsigned char> opusPacket = opusCodec.encode_float(frameData, frameSamples);
            //
            // if (!opusPacket.empty()) {
            //     // (3) Écriture du paquet Opus dans le fichier .opus
            //     encodedOpusFileHandler.write(opusPacket);
            //
            //     // (4) Optionnel : décodage pour vérification
            //     std::vector<float> decodedFrame = opusCodec.decode_float(opusPacket);
            //     if (!decodedFrame.empty())
            //         decodedWavFileHandler.write(decodedFrame, decodedFrame.size());
            // } else {
            //     // Gestion d'erreur d'encodage : éventuellement journaliser ou notifier
            // }
        } else {
            // Si pas assez d'échantillons, on attend un peu
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}


void WebRTCAudioService::startAudioThread() {
    encodingThread = std::thread(&WebRTCAudioService::processingThreadFunction, this);
    threadRunning = true;
}

void WebRTCAudioService::stopAudioThread() {
    threadRunning = false;
    if (encodingThread.joinable())
        encodingThread.join();
}

void WebRTCAudioService::onRTCStateChanged(const RTCStateChangeEvent &event) {
    if (event.state == rtc::PeerConnection::State::Connected && !threadRunning) {
        createFiles();
        startAudioThread();
    } else {
        stopAudioThread();
    }
}


void WebRTCAudioService::createFiles() {
    vanillaWavFile.create("1_base_audio.wav");
    encodedOpusFileHandler.create("2_encoded_opus_audio.opus");
    decodedWavFileHandler.create("1_decoded_opus_audio.wav");
}

void WebRTCAudioService::finalizeFiles() {
    vanillaWavFile.close();
    encodedOpusFileHandler.close();
    decodedWavFileHandler.close();
}
