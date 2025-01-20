#include "WebRTCAudioService.h"

int SAMPLE_RATE_2 = 44100;
// int SAMPLE_RATE_2 = 24000;
int SAMPLE_RATE = 48000;
int CHANNELS = 2;
// int CHANNELS = 2;
int BITRATE = 64000;


const std::string wavFilename = FileUtils::generateTimestampedFilename("output", "wav");

WebRTCAudioService::WebRTCAudioService():
    opusEncoder(SAMPLE_RATE, CHANNELS, BITRATE),
    opusDecoder(SAMPLE_RATE, CHANNELS)
{
    wavFile = initializeWavFile(wavFilename, SAMPLE_RATE_2, CHANNELS);
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

        // Récupérer les données PCM de la file d'attente
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this]() { return !audioQueue.empty() || stopThread; });
            if (stopThread) break;

            pcmData = std::move(audioQueue.front());
            audioQueue.pop();
        }
        for (float sample : pcmData) {
            if (sample < -1.0f || sample > 1.0f) {
                std::cerr << "Sample out of range: " << sample << std::endl;
            }
            int16_t intSample = static_cast<int16_t>(std::clamp(sample * 32767.0f, -32768.0f, 32767.0f));
            wavFile.write(reinterpret_cast<const char*>(&intSample), sizeof(int16_t));
        }
        if (audioTrack) {
            try {
                juce::Logger::outputDebugString("Sending audio data: " + std::to_string(pcmData.size()) + " bytes");

                // std::vector<float> pcmDataLocal = opusDecoder.decode(encodedData);
                // appendWavData(wavFilename, pcmData);

                // audioTrack->send(reinterpret_cast<const std::byte *>(encodedData.data()), encodedData.size());
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
    try {
        finalizeWavFile(std::move(wavFile));
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
