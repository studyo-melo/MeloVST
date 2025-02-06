#pragma once
#include "AudioAppPlayer.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include "../Utils/FileUtils.h"
#include "../Utils/VectorUtils.h"
#include <stdio.h>
#include <vector>
#include "../Events/EventListener.h"
#include "../Events/EventManager.h"
#include "../Utils/AudioSettings.h"

class AudioAppPlayer : public juce::AudioAppComponent, public EventListener {
public:
    AudioAppPlayer();
    ~AudioAppPlayer() override;

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
