#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

struct AudioBlockProcessedEvent {
    juce::AudioBuffer<float>& buffer;
    int totalNumInputChannels;
    int totalNumOutputChannels;
};

struct LoginEvent {
    juce::String accessToken;
};

struct LogoutEvent {};