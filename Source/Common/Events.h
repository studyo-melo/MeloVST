#pragma once

#include "../Models/Session.h"
#include <rtc/rtc.hpp>
#include "../ThirdParty/json.hpp"

struct AudioBlockProcessedEvent {
    const std::vector<float> data;
    int numChannels;
    int numSamples;
    double sampleRate;
};

struct AudioBlockSentEvent {
    const std::vector<float> data;
};

struct LoginEvent {
    juce::String accessToken;
};

struct LogoutEvent {};
struct OngoingSessionChangedEvent {
    PopulatedSession ongoingSession;
};

struct MessageWsReceivedEvent {
    std::string type;
    nlohmann::json data;
};

struct RTCStateChangeEvent {
    rtc::PeerConnection::State state;
    rtc::PeerConnection::IceState iceState;
    rtc::PeerConnection::SignalingState signalingState;
};