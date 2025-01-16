#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include "../Models/Session.h"
#include <rtc/rtc.hpp>

struct AudioBlockProcessedEvent {
    juce::AudioBuffer<float>& buffer;
    int totalNumInputChannels;
    int totalNumOutputChannels;
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