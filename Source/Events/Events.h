#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include "../Models/Session.h"
#include <rtc/rtc.hpp>

struct AudioBlockProcessedEvent {
    const std::vector<int16_t> data;
    int numChannels;
    int numSamples;
    double sampleRate;
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