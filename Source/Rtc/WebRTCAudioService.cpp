#include "WebRTCAudioService.h"

WebRTCAudioService::WebRTCAudioService()
    : opusEncoder(48000, 2, 64000) { // Configuration : 48 kHz, stéréo, 64 kbps
}

WebRTCAudioService::~WebRTCAudioService() {
    stopAudioThread();
}

void WebRTCAudioService::pushAudioBuffer(const float* data, int numSamples) {
    // Convertir les échantillons float en PCM int16_t
    std::vector<int16_t> pcmData(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        pcmData[i] = static_cast<int16_t>(data[i] * 32767.0f); // Conversion float -> PCM 16-bit
    }

    // Ajouter les données PCM à la file d'attente thread-safe
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        audioQueue.push(std::move(pcmData));
    }
    queueCondition.notify_one();
}

void WebRTCAudioService::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    if (!audioTrack || !audioTrack->isOpen()) {
        return;
    }
    pushAudioBuffer(event.data, event.numSamples);
}

void WebRTCAudioService::startAudioThread() {
    juce::Logger::outputDebugString("Starting audio thread");
    stopAudioThread();
    stopThread = false;
    audioThread = std::thread([this]() {
        sendAudioData();
    });
}

void WebRTCAudioService::sendAudioData() {
    const int frameSize = 960; // Taille de trame pour 20ms @ 48kHz (960 échantillons par canal)
    const int maxPacketSize = 520; // Taille maximale du paquet RTP (en octets)

    while (!stopThread) {
        std::vector<int16_t> pcmData;

        // Récupérer les données PCM de la file d'attente
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this]() { return !audioQueue.empty() || stopThread; });
            if (stopThread) break;

            pcmData = std::move(audioQueue.front());
            audioQueue.pop();
        }

        // Diviser les données PCM en trames de taille appropriée
        for (size_t offset = 0; offset < pcmData.size(); offset += frameSize * 2) {
            auto frameEnd = std::min(offset + frameSize * 2, pcmData.size());
            std::vector<int16_t> frame(pcmData.begin() + offset, pcmData.begin() + frameEnd);

            // Convertir la trame int16_t en float
            std::vector<float> floatFrame(frame.size());
            for (size_t i = 0; i < frame.size(); ++i) {
                floatFrame[i] = static_cast<float>(frame[i]) / 32767.0f; // Normaliser en [-1.0, 1.0]
            }

            // Encoder la trame avec Opus
            std::vector<uint8_t> encodedData;
            try {
                encodedData = opusEncoder.encode(floatFrame.data(), frameSize); // Utilise floatFrame.data()
                if (encodedData.size() > maxPacketSize) {
                    juce::Logger::outputDebugString("Encoded packet exceeds maximum size!");
                    continue;
                }
            } catch (const std::exception &e) {
                juce::Logger::outputDebugString("Error encoding audio frame: " + std::string(e.what()));
                continue;
            }

            // Envoyer le paquet encodé via le canal WebRTC
            if (audioTrack) {
                try {
                    audioTrack->send(reinterpret_cast<const std::byte *>(encodedData.data()), encodedData.size());
                } catch (const std::exception &e) {
                    juce::Logger::outputDebugString("Error sending audio data: " + std::string(e.what()));
                }
            }
        }
    }
}


void WebRTCAudioService::stopAudioThread() {
    juce::Logger::outputDebugString("Stopping audio thread");
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopThread = true;
    }
    queueCondition.notify_all();
    if (audioThread.joinable()) {
        audioThread.join();
    }
}

void WebRTCAudioService::onRTCStateChanged(const RTCStateChangeEvent &event) {
    if (event.state == rtc::PeerConnection::State::Connected) {
        startAudioThread();
    }
}
