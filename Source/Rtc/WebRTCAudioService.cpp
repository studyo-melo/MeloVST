#include "WebRTCAudioService.h"

int SAMPLE_RATE = 48000;
int CHANNELS = 2;
int BITRATE = 64000;


const std::string wavFilename = FileUtils::generateTimestampedFilename("output", "wav");

WebRTCAudioService::WebRTCAudioService():
    opusEncoder(SAMPLE_RATE, CHANNELS, BITRATE),
    opusDecoder(SAMPLE_RATE, CHANNELS),
    stopThread(false)
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

void WebRTCAudioService::pushAudioBuffer(const float* data, int numSamples) {
    // Convertir les échantillons float en PCM int16_t
    std::vector<int16_t> pcmData(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        pcmData[i] = static_cast<int16_t>(std::clamp(data[i], -1.0f, 1.0f) * 32767.0f);
    }
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
            if (frame.size() < frameSize * 2) {
                frame.resize(frameSize * 2, 0);
            }

            std::vector<float> floatFrame(frameSize * 2);
            for (size_t i = 0; i < frame.size(); ++i) {
                floatFrame[i] = static_cast<float>(frame[i]) / 32768.0f;
            }

            std::vector<uint8_t> encodedData;
            try {
                encodedData = opusEncoder.encode(floatFrame.data(), frameSize);
                if (encodedData.size() > maxPacketSize) {
                    juce::Logger::outputDebugString("Encoded packet exceeds maximum size!");
                    continue;
                }
                if (encodedData.size() < 64) {
                    continue;
                }
            } catch (const std::exception &e) {
                juce::Logger::outputDebugString("Error encoding audio frame: " + std::string(e.what()));
                continue;
            }

            if (audioTrack) {
                try {
                    juce::Logger::outputDebugString("Sending audio data: " + std::to_string(encodedData.size()) + " bytes");

                    // Décoder les données pour les ajouter au fichier WAV
                    std::vector<float> pcmDataLocal = opusDecoder.decode(encodedData);
                    // Ajouter les données décodées au fichier WAV
                    appendWavData(wavFilename, pcmDataLocal);

                    // Envoyer les données via WebRTC
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
    try {
        finalizeWavFile(wavFilename);
    } catch (const std::exception &e) {
        juce::Logger::outputDebugString("Error finalizing WAV file: " + std::string(e.what()));
    }
}

void WebRTCAudioService::onRTCStateChanged(const RTCStateChangeEvent &event) {
    if (event.state == rtc::PeerConnection::State::Connected) {
        startAudioThread();
    }
}
