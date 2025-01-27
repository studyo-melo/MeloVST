#include "WebRTCAudioService.h"

#include "../Debug/DebugRTPWrapper.h"
#include "../Utils/AudioSettings.h"
#include "../Utils/AudioUtils.h"

WebRTCAudioService::WebRTCAudioService():
opusCodec(10, 2, 48000),
resampler(44100, 48000, 2)
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
        std::vector<int16_t> pcmData;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this]() { return !audioQueue.empty() || stopThread; });
            if (stopThread) break;

            pcmData = std::move(audioQueue.front());
            audioQueue.pop();
        }
        if (pcmData.empty()) {
            continue;
        }

        if (audioTrack) {
            try {
                std::vector<int16_t> resampledAudioBlock = resampler.resampleFromInt16(pcmData);
                std::vector<int8_t> opusEncodedAudioBlock = opusCodec.encode(pcmData);
                if (opusEncodedAudioBlock.empty()) {
                    juce::Logger::outputDebugString("Empty opus encoded audio block");
                    continue;
                }
                auto rtpPacket = RTPWrapper::createRTPPacket(opusEncodedAudioBlock, seqNum++, timestamp, ssrc);
                timestamp += opusEncodedAudioBlock.size() / 2;
                juce::Logger::outputDebugString("Sending audio data: " + std::to_string(rtpPacket.size()) + " bytes");
                DebugRTPWrapper::debugPacket(rtpPacket);

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
