#include "WebRTCAudioService.h"

#include "../Debug/DebugRTPWrapper.h"
#include "../Utils/AudioSettings.h"
#include "../Utils/AudioUtils.h"

WebRTCAudioService::WebRTCAudioService(): opusCodec(), opusEncoder(48000, 2, 10)
{
}

WebRTCAudioService::~WebRTCAudioService() {
    stopAudioThread();
}

void WebRTCAudioService::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    if (!audioTrack || !audioTrack->isOpen()) {
        return;
    }
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        audioQueue.push(event.data);
    }
    queueCondition.notify_one();
}

void WebRTCAudioService::sendAudioData() {
    while (!stopThread) {
        std::vector<float> pcmData;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this]() { return !audioQueue.empty() || stopThread; });
            if (stopThread) break;

            pcmData = std::move(audioQueue.front());
            audioQueue.pop();
        }
        if (pcmData.empty()) {
            return;
        }

        if (audioTrack) {
            try {
                std::vector<int16_t> audioBlockInt16(pcmData.size());
                for (size_t i = 0; i < pcmData.size(); ++i) {
                    audioBlockInt16[i] = static_cast<int16_t>(pcmData[i] * 32767.0f);
                }
                std::vector<uint8_t> opusEncodedAudioBlock; // Déclaration de la variable à l'extérieur
                opusEncoder.Encode(std::move(audioBlockInt16), [&opusEncodedAudioBlock](std::vector<uint8_t>&& encodedData) {
                    opusEncodedAudioBlock = std::move(encodedData); // Capturer par référence
                });
                auto rtpPacket = RTPWrapper::createRTPPacket(opusEncodedAudioBlock, seqNum++, timestamp, ssrc);
                juce::Logger::outputDebugString("Sending audio data: " + std::to_string(rtpPacket.size()) + " bytes");
                audioTrack->send(reinterpret_cast<const std::byte *>(rtpPacket.data()), rtpPacket.size());
            } catch (const std::exception &e) {
                juce::Logger::outputDebugString("Error sending audio data: " + std::string(e.what()));
            }
        }
    }
}


void WebRTCAudioService::startAudioThread() {
    if (audioThreadRunning) {
        return;
    }
    juce::Logger::outputDebugString("Starting audio thread");
    stopThread = false;
    audioThreadRunning = true;
    audioThread = std::thread([this]() {
        sendAudioData();
    });
}

void WebRTCAudioService::stopAudioThread() {
    if (!audioThreadRunning) {
        return;
    }
    juce::Logger::outputDebugString("Stopping audio thread");
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopThread = true;
        audioThreadRunning = false;
    }
    queueCondition.notify_all();
    if (audioThread.joinable()) {
        audioThread.join();
    }
}

void WebRTCAudioService::onRTCStateChanged(const RTCStateChangeEvent &event) {
    if (event.state == rtc::PeerConnection::State::Connected && !audioThreadRunning) {
        startAudioThread();
    }
    else {
        stopAudioThread();
    }
}
