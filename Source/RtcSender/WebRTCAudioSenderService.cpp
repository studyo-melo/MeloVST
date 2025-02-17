#include "WebRTCAudioSenderService.h"
#include "../Debug/DebugRTPWrapper.h"
#include "../AudioSettings.h"
#include "../Common/EventManager.h"
#include <rtc/rtc.hpp>
#include "../Api/SocketRoutes.h"

WebRTCAudioSenderService::WebRTCAudioSenderService(): WebRTCSenderConnexionHandler(WsRoute::GetOngoingSessionRTCInstru),
                                                      opusEncoder(AudioSettings::getInstance().getOpusSampleRate(),
                                                                  AudioSettings::getInstance().getNumChannels(),
                                                                  AudioSettings::getInstance().getLatency(),
                                                                  AudioSettings::getInstance().getOpusBitRate()),
                                                      resampler(AudioSettings::getInstance().getSampleRate(),
                                                                AudioSettings::getInstance().getOpusSampleRate(),
                                                                AudioSettings::getInstance().getNumChannels()),
                                                      circularBuffer(
                                                          AudioSettings::getInstance().getSampleRate() *
                                                          AudioSettings::getInstance().getNumChannels()) {
}

WebRTCAudioSenderService::~WebRTCAudioSenderService() {
    stopAudioThread();
}

void WebRTCAudioSenderService::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    juce::ScopedLock sl(dequeLock);
    // Ajoute l'événement dans une file de priorité
    audioEventQueue.push(std::make_shared<AudioBlockProcessedEvent>(event));
}
void WebRTCAudioSenderService::processingThreadFunction() {
    const int sampleRate = AudioSettings::getInstance().getSampleRate();
    const int frameDurationMs = AudioSettings::getInstance().getLatency(); // par exemple 20 ms
    const int totalFrameSamples = static_cast<int>(sampleRate * frameDurationMs / 1000.0); // traitement mono

    std::vector<float> accumulatedData;
    while (threadRunning) {
        {
            // Accumuler tous les événements disponibles dans la file triée
            juce::ScopedLock sl(dequeLock);
            while (!audioEventQueue.empty()) {
                auto event = audioEventQueue.top();
                audioEventQueue.pop();
                // Ajouter les données de l'événement à notre tampon d'accumulation
                accumulatedData.insert(accumulatedData.end(),
                                       event->data.begin(),
                                       event->data.end());
            }
        } // Fin du verrou

        // Vérifier si on a assez d'échantillons pour une trame complète
        if (accumulatedData.size() < static_cast<size_t>(totalFrameSamples)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        // Extraire les échantillons nécessaires pour la trame
        std::vector<float> frameData(accumulatedData.begin(),
                                     accumulatedData.begin() + totalFrameSamples);
        // Supprimer les échantillons extraits du tampon d'accumulation
        accumulatedData.erase(accumulatedData.begin(),
                              accumulatedData.begin() + totalFrameSamples);

        // Encoder la trame avec Opus
        std::vector<unsigned char> opusPacket = opusEncoder.encode_float(frameData, totalFrameSamples);
        if (opusPacket.empty())
            continue;

        sendOpusPacket(opusPacket, totalFrameSamples);
    }
}

void WebRTCAudioSenderService::sendOpusPacket(const std::vector<unsigned char> &opusPacket, int totalFrameSamples) {
    try {
        auto rtpPacket = RTPWrapper::createRTPPacket(opusPacket, seqNum++, timestamp, ssrc);
        // Mettre à jour le timestamp (ici, vous pouvez réutiliser event.timestamp ou incrémenter)
        timestamp += totalFrameSamples;
        juce::Logger::outputDebugString("Sending audio data: " + std::to_string(rtpPacket.size()) + " bytes");
        audioTrack->send(reinterpret_cast<const std::byte *>(rtpPacket.data()), rtpPacket.size());
    } catch (const std::exception &e) {
        juce::Logger::outputDebugString("Error sending audio data: " + std::string(e.what()));
    }
}

void WebRTCAudioSenderService::startAudioThread() {
    threadRunning = true;
    encodingThread = std::thread(&WebRTCAudioSenderService::processingThreadFunction, this);
}

void WebRTCAudioSenderService::stopAudioThread() {
    threadRunning = false;
    if (encodingThread.joinable())
        encodingThread.join();
}


void WebRTCAudioSenderService::onRTCStateChanged(const RTCStateChangeEvent &event) {
    if (event.state == rtc::PeerConnection::State::Connected && !threadRunning) {
        startAudioThread();
    } else {
        stopAudioThread();
    }
}
