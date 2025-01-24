#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Events/EventListener.h"
#include "../Events/EventManager.h"
#include "../Rtc/OpusCodecWrapper.h"
#include "../Utils/AudioSettings.h"

class AudioBlockPlayer : public juce::AudioAppComponent, EventListener
{
public:
    AudioBlockPlayer(): opusCodec()
    {
        setAudioChannels(0, 2);
        EventManager::getInstance().addListener(this);
    }

    ~AudioBlockPlayer() override
    {
        shutdownAudio();
        EventManager::getInstance().removeListener(this);
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        currentSampleRate = sampleRate;
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        if (audioBlock.empty())
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        auto* leftChannel = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* rightChannel = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

        if (/* DISABLES CODE */ (true)) {
            auto encodedAudioBlock = opusCodec.encode(audioBlock.data());
            auto decodedAudioBlock = opusCodec.decode(encodedAudioBlock);
            for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                if (currentSampleIndex >= decodedAudioBlock.size())
                    currentSampleIndex = 0; // Boucle sur le bloc audio

                float currentSample = decodedAudioBlock[currentSampleIndex++];
                leftChannel[sample] = currentSample;
                rightChannel[sample] = currentSample;
            }
        }
        else {
            for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                if (currentSampleIndex >= audioBlock.size())
                    currentSampleIndex = 0; // Boucle sur le bloc audio

                float currentSample = audioBlock[currentSampleIndex++];
                leftChannel[sample] = currentSample;
                rightChannel[sample] = currentSample;
            }
        }
    }

    void releaseResources() override
    {
        audioBlock.clear();
    }

    void onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) override {
        audioBlock.resize(event.data.size());
        audioBlock = event.data;
        currentNumSamples = event.numSamples;
    }

private:
    std::vector<float> audioBlock; // Contient les échantillons audio
    int currentNumSamples = 0; // Nombre d'échantillons dans le bloc
    size_t currentSampleIndex = 0; // Position actuelle dans le bloc
    double currentSampleRate = 0.0;

    OpusCodecWrapper opusCodec;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioBlockPlayer)
};