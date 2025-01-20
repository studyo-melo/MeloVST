#include "WebRTCAudioService.h"

int SAMPLE_RATE = 48000;
int CHANNELS = 2;
int BITRATE = 64000;


const std::string wavFilename = FileUtils::generateTimestampedFilename("output", "wav");

WebRTCAudioService::WebRTCAudioService():
    opusEncoder(SAMPLE_RATE, CHANNELS, BITRATE),
    opusDecoder(SAMPLE_RATE, CHANNELS)
{
    try {
        initializeWavFile(wavFilename, SAMPLE_RATE, CHANNELS);
    } catch (const std::exception &e) {
        juce::Logger::outputDebugString("Error initializing WAV file: " + std::string(e.what()));
        return;
    }
}

WebRTCAudioService::~WebRTCAudioService() {
    stopAudioThread();
}

void WebRTCAudioService::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    if (!audioTrack || !audioTrack->isOpen()) {
        return;
    }
    {
        std::vector<float> audioData(event.data, event.data + event.numSamples);
        std::unique_lock<std::mutex> lock(queueMutex);
        audioQueue.push(std::move(audioData));
    }
    queueCondition.notify_one();
}

void WebRTCAudioService::sendAudioData() {
    while (!stopThread) {
        std::vector<float> pcmData;

        // Récupérer les données PCM de la file d'attente
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

                // std::vector<float> pcmDataLocal = opusDecoder.decode(encodedData);
                appendWavData(wavFilename, pcmData);

                // audioTrack->send(reinterpret_cast<const std::byte *>(encodedData.data()), encodedData.size());
            } catch (const std::exception &e) {
                juce::Logger::outputDebugString("Error sending audio data: " + std::string(e.what()));
            }
        }
    }
}


void WebRTCAudioService::startAudioThread() {
    juce::Logger::outputDebugString("Starting audio thread");
    stopThread = false;
    audioThreadRunning = true;
    audioThread = std::thread([this]() {
        sendAudioData();
    });
}

void WebRTCAudioService::stopAudioThread() {
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
    try {
        finalizeWavFile(wavFilename);
    } catch (const std::exception &e) {
        juce::Logger::outputDebugString("Error finalizing WAV file: " + std::string(e.what()));
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
