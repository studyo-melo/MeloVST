#include "WebRTCAudioReceiverService.h"
#include "../Debug/DebugRTPWrapper.h"
#include "../AudioSettings.h"
#include "../Common/EventManager.h"
#include <rtc/rtc.hpp>
#include "../Api/SocketRoutes.h"
#include "../Utils/VectorUtils.h"

WebRTCAudioReceiverService::WebRTCAudioReceiverService(): WebRTCReceiverConnexionHandler(
                                                              WsRoute::GetOngoingSessionRTCVoice),
                                                          opusCodec(AudioSettings::getInstance().getOpusSampleRate(),
                                                                    AudioSettings::getInstance().getNumChannels(),
                                                                    AudioSettings::getInstance().getLatency(),
                                                                    AudioSettings::getInstance().getOpusBitRate()),
                                                          resampler(AudioSettings::getInstance().getSampleRate(),
                                                                    AudioSettings::getInstance().getOpusSampleRate(),
                                                                    AudioSettings::getInstance().getNumChannels()),
                                                          circularBuffer(
                                                              AudioSettings::getInstance().getSampleRate() *
                                                              AudioSettings::getInstance().getNumChannels()) {
}

WebRTCAudioReceiverService::~WebRTCAudioReceiverService() {}

void WebRTCAudioReceiverService::onAudioBlockReceived(const AudioBlockReceivedEvent &event) {
    juce::Logger::outputDebugString("Audio block received");
    rtc::message_variant audioBlock = event.data;
    auto res = VectorUtils::convertMessageToUChar(audioBlock);
    if (!res.empty()) {
        std::vector<float> decodedPacket = opusCodec.decode_float(res);
        if (decodedPacket.empty()) {
            return;
        }
        auto floatPacket = VectorUtils::convertFloatInt8ToFloat(decodedPacket.data(), decodedPacket.size());
        EventManager::getInstance().notifyOnAudioBlockReceivedDecoded(AudioBlockReceivedDecodedEvent{floatPacket});
        res.clear();
    }
}
