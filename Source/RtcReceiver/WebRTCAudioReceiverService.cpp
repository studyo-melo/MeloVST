#include "WebRTCAudioReceiverService.h"
#include "../Debug/DebugRTPWrapper.h"
#include "../AudioSettings.h"
#include "../Common/EventManager.h"
#include <rtc/rtc.hpp>
#include "../Api/SocketRoutes.h"

WebRTCAudioReceiverService::WebRTCAudioReceiverService(): WebRTCReceiverConnexionHandler(WsRoute::GetOngoingSessionRTCVoice, rtc::Description::Direction::RecvOnly),
                                                      opusCodec(AudioSettings::getInstance().getOpusSampleRate(), AudioSettings::getInstance().getNumChannels(), AudioSettings::getInstance().getLatency(), AudioSettings::getInstance().getOpusBitRate()),
                                                      resampler(AudioSettings::getInstance().getSampleRate(), AudioSettings::getInstance().getOpusSampleRate(), AudioSettings::getInstance().getNumChannels()),
                                                      circularBuffer(AudioSettings::getInstance().getSampleRate() * AudioSettings::getInstance().getNumChannels()) {
}

WebRTCAudioReceiverService::~WebRTCAudioReceiverService() {
    stopAudioThread();
}

void WebRTCAudioReceiverService::onAudioBlockReceived(const AudioBlockReceivedEvent &event) {
      juce::Logger::outputDebugString("Audio block received");
//    std::vector<float> audioBlock = event.data;
//
//    // On vérifie que le buffer audio (audioBlock) contient des données
//    if (!audioBlock.empty()) {
//        {
//            const int numSamples = static_cast<int>(audioBlock.size());
//            juce::ScopedLock sl(circularBufferLock);
//            circularBuffer.pushSamples(audioBlock.data(), numSamples);
//        }
//        // On vide le buffer temporaire pour éviter une réécriture multiple
//        audioBlock.clear();
//    }
}

void WebRTCAudioReceiverService::processingThreadFunction() {
    const int frameSamples = static_cast<int>(AudioSettings::getInstance().getSampleRate() * AudioSettings::getInstance().getLatency() / 1000.0); // 48000 * 20/1000 = 960
    const int totalFrameSamples = frameSamples * AudioSettings::getInstance().getNumChannels(); // Pour un signal interleaved
    while (threadRunning) {
        bool frameAvailable = false;
        std::vector<unsigned char> frameData(totalFrameSamples); {
            juce::ScopedLock sl(circularBufferLock);
            if (circularBuffer.getNumAvailableSamples() >= totalFrameSamples) {
                circularBuffer.popSamples(frameData.data(), totalFrameSamples);
                frameAvailable = true;
            }
        }

        if (frameAvailable) {
            std::vector<float> decodedPacket = opusCodec.decode_float(frameData);
            if (decodedPacket.empty()) {
                return;
            }
            EventManager::getInstance().notifyOnAudioBlockReceivedDecoded(AudioBlockReceivedDecodedEvent{decodedPacket});
        } else {
            // Si pas assez d'échantillons, on attend un peu
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}


void WebRTCAudioReceiverService::startAudioThread() {
    encodingThread = std::thread(&WebRTCAudioReceiverService::processingThreadFunction, this);
    threadRunning = true;
}

void WebRTCAudioReceiverService::stopAudioThread() {
    threadRunning = false;
    if (encodingThread.joinable())
        encodingThread.join();
}

void WebRTCAudioReceiverService::onRTCStateChanged(const RTCStateChangeEvent &event) {
    if (event.state == rtc::PeerConnection::State::Connected && !threadRunning) {
        startAudioThread();
    } else {
        stopAudioThread();
    }
}
