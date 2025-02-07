#include "WebRTCAudioSenderService.h"
#include "../Debug/DebugRTPWrapper.h"
#include "../AudioSettings.h"
#include "../Common/EventManager.h"
#include <rtc/rtc.hpp>

WebRTCAudioSenderService::WebRTCAudioSenderService(): WebRTCConnexionHandler(WsRoute::GetOngoingSessionRTC,
                                                                             rtc::Description::Direction::SendOnly),
                                                      opusCodec(AudioSettings::getInstance().getOpusSampleRate(), AudioSettings::getInstance().getNumChannels(), AudioSettings::getInstance().getLatency(), AudioSettings::getInstance().getOpusBitRate()),
                                                      resampler(AudioSettings::getInstance().getSampleRate(), AudioSettings::getInstance().getOpusSampleRate(), AudioSettings::getInstance().getNumChannels()),
                                                      circularBuffer(AudioSettings::getInstance().getSampleRate() * AudioSettings::getInstance().getNumChannels()) {
}

WebRTCAudioSenderService::~WebRTCAudioSenderService() {
    stopAudioThread();
}

void WebRTCAudioSenderService::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    std::vector<float> audioBlock = event.data;

    // On vérifie que le buffer audio (audioBlock) contient des données
    if (!audioBlock.empty()) {
        {
            const int numSamples = static_cast<int>(audioBlock.size());
            juce::ScopedLock sl(circularBufferLock);
            circularBuffer.pushSamples(audioBlock.data(), numSamples);
        }
        // On vide le buffer temporaire pour éviter une réécriture multiple
        audioBlock.clear();
    }
}

void WebRTCAudioSenderService::processingThreadFunction() {
    // Calcul du nombre d'échantillons par canal pour une trame de 20 ms
    const int frameSamples = static_cast<int>(AudioSettings::getInstance().getSampleRate() * AudioSettings::getInstance().getLatency() / 1000.0); // 48000 * 20/1000 = 960
    const int totalFrameSamples = frameSamples * AudioSettings::getInstance().getNumChannels(); // Pour un signal interleaved
    while (threadRunning) {
        bool frameAvailable = false;
        std::vector<float> frameData(totalFrameSamples); {
            juce::ScopedLock sl(circularBufferLock);
            if (circularBuffer.getNumAvailableSamples() >= totalFrameSamples) {
                circularBuffer.popSamples(frameData.data(), totalFrameSamples);
                frameAvailable = true;
            }
        }

        if (frameAvailable) {
            EventManager::getInstance().notifyOnAudioBlockSent(AudioBlockSentEvent{frameData});

            std::vector<unsigned char> opusPacket = opusCodec.encode_float(frameData, frameSamples);
            if (opusPacket.empty()) {
                return;
            }
            if (audioTrack) {
                try {
                    auto rtpPacket = RTPWrapper::createRTPPacket(opusPacket, seqNum++, timestamp, ssrc);
                    timestamp += frameSamples;
                    juce::Logger::outputDebugString(
                        "Sending audio data: " + std::to_string(rtpPacket.size()) + " bytes");
                    DebugRTPWrapper::debugPacket(rtpPacket);

                    audioTrack->send(reinterpret_cast<const std::byte *>(rtpPacket.data()), rtpPacket.size());
                } catch (const std::exception &e) {
                    juce::Logger::outputDebugString("Error sending audio data: " + std::string(e.what()));
                }
            }
        } else {
            // Si pas assez d'échantillons, on attend un peu
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}


void WebRTCAudioSenderService::startAudioThread() {
    encodingThread = std::thread(&WebRTCAudioSenderService::processingThreadFunction, this);
    threadRunning = true;
}

void WebRTCAudioSenderService::stopAudioThread() {
    threadRunning = false;
    if (encodingThread.joinable())
        encodingThread.join();
}

void WebRTCAudioSenderService::onRTCStateChanged(const RTCStateChangeEvent &event) {
    if (event.state == rtc::PeerConnection::State::Connected && !threadRunning) {
        createFiles();
        startAudioThread();
    } else {
        stopAudioThread();
    }
}


void WebRTCAudioSenderService::createFiles() {
    return;
}

void WebRTCAudioSenderService::finalizeFiles() {
    return;
}
