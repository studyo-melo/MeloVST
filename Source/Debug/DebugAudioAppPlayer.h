#pragma once
#include "DebugAudioAppPlayer.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>
#include "../Common/EventListener.h"

class DebugAudioAppPlayer final : public juce::AudioAppComponent, public EventListener {
public:
    DebugAudioAppPlayer();
    ~DebugAudioAppPlayer() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;
    void releaseResources() override;

    // Callback custom lorsque le bloc audio est prêt à être traité (issu d'un EventManager, par exemple)
    void onAudioBlockSent(const AudioBlockSentEvent &event) override;

private:
    std::vector<float> audioBlock;
    int currentNumSamples = 0;
    int currentSampleIndex = 0;

    double currentSampleRate = 0;
};
