#include "WebRTCAudioService.h"

WebRTCAudioService::~WebRTCAudioService() {
    WebRTCAudioService::stopAudioThread();
}

void WebRTCAudioService::pushAudioBuffer(const float* data, int numSamples) {
    std::vector<int16_t> pcmData(numSamples);
    for (int i = 0; i < numSamples; ++i)
    {
        pcmData[i] = static_cast<int16_t>(data[i] * 32767.0f); // Normaliser si nécessaire
    }

    // Ajouter les données PCM dans la queue thread-safe
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
};

void WebRTCAudioService::startAudioThread() {
    juce::Logger::outputDebugString("Starting audio thread");
    stopAudioThread();
    stopThread = false;
    audioThread = std::thread([this]() {
        sendAudioData();
    });
}

void WebRTCAudioService::sendAudioData() {
    while (!stopThread) {
        std::vector<int16_t> pcmData; {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this]() { return !audioQueue.empty() || stopThread; });
            if (stopThread) break;

            pcmData = std::move(audioQueue.front());
            audioQueue.pop();
        }

        juce::Logger::outputDebugString("Sending audio data");
        if (audioTrack) {
            try {
                audioTrack->send(reinterpret_cast<const std::byte *>(pcmData.data()), pcmData.size() * sizeof(int16_t));
            }
            catch (const std::exception& e) {
                juce::Logger::outputDebugString("Error sending audio data" + std::string(e.what()));
            }
        }
    }
}

void WebRTCAudioService::stopAudioThread() {
    juce::Logger::outputDebugString("Stopping audio thread"); {
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
