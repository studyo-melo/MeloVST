#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include "../Models/Session.h"
#include <rtc/rtc.hpp>
#include <yangutil/yangavctype.h>

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
    YangRtcConnectionState state;
    YangIceCandidateState iceState;
    YangRequestType signalingState;
};