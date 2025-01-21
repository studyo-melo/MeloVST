#include "WebRTCAudioService.h"

#include "../Utils/AudioSettings.h"

WebRTCAudioService::WebRTCAudioService():
    opusEncoder(AudioSettings::getInstance().getOpusSampleRate(), AudioSettings::getInstance().getNumChannels(), AudioSettings::getInstance().getBitDepth()),
    opusDecoder(AudioSettings::getInstance().getOpusSampleRate(), AudioSettings::getInstance().getNumChannels()) {
}

WebRTCAudioService::~WebRTCAudioService() {
    stopAudioThread();
}

void WebRTCAudioService::createFile() {
    const std::string wavFilename = FileUtils::generateTimestampedFilename("output", "wav");
    wavFile = FileUtils::initializeWavFile(wavFilename);
}


void WebRTCAudioService::finalizeFile() {
    FileUtils::finalizeWavFile(std::move(wavFile));
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

        if (audioTrack) {
            try {
                juce::Logger::outputDebugString("Sending audio data: " + std::to_string(pcmData.size()) + " bytes");
                FileUtils::appendWavData(wavFile, pcmData);
                auto encodedData = opusEncoder.encode(pcmData.data(), pcmData.size());
                audioTrack->send(reinterpret_cast<const std::byte *>(encodedData.data()), encodedData.size());
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
