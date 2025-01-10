#pragma once

#include <iostream>
#include <rtc/rtc.hpp>
#include "../Utils/VectorUtils.h"
#include <juce_core/juce_core.h>

class MeloWebRTCServerService {
public:
    MeloWebRTCServerService();

    void handleAnswer(const std::string& sdp) const;
    void sendAudioData(const float* data, int size) const;

private:
    std::shared_ptr<rtc::PeerConnection> peerConnection;
    std::shared_ptr<rtc::DataChannel> dataChannel;
};
