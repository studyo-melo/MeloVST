#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Events/EventListener.h"
#include "../Events/EventManager.h"
#include "../Rtc/OpusCodecWrapper.h"
#include "../Utils/AudioSettings.h"

class AudioAppPlayer : public juce::AudioAppComponent, public EventListener {
public:
    AudioAppPlayer();

    ~AudioAppPlayer();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    void releaseResources() override;

    void onAudioBlockProcessedEvent(const AudioBlockProcessedEvent& event) override;

private:
    std::vector<float> audioBlock; // Contient les échantillons audio
    int currentNumSamples = 0; // Nombre d'échantillons dans le bloc
    size_t currentSampleIndex = 0; // Position actuelle dans le bloc
    double currentSampleRate = 0.0;

    OpusCodecWrapper opusCodec;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioAppPlayer)
};
