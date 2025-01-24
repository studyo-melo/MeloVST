#pragma once
#include <vector>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>


namespace AudioUtils {
    inline std::vector<float> resampleAudioJUCE(const std::vector<float>& inputData,
                                                int inputSampleRate,
                                                int outputSampleRate,
                                                int numChannels)
    {
        if (inputSampleRate == outputSampleRate) {
            return inputData; // Pas de conversion nécessaire
        }

        // Calcul du ratio
        const double ratio = static_cast<double>(outputSampleRate) / inputSampleRate;

        // Préparer le buffer d'entrée et de sortie
        const size_t inputFrames = inputData.size() / numChannels;
        const size_t outputFrames = static_cast<size_t>(std::ceil(inputFrames * ratio));

        std::vector<float> outputData(outputFrames * numChannels, 0.0f);

        // Rééchantillonneur par canal
        juce::LagrangeInterpolator interpolator;

        for (int channel = 0; channel < numChannels; ++channel) {
            const float* inputChannelData = inputData.data() + channel;
            float* outputChannelData = outputData.data() + channel;

            interpolator.reset(); // Réinitialiser le rééchantillonneur
            interpolator.process(ratio, inputChannelData, outputChannelData, static_cast<int>(outputFrames));
        }

        return outputData;
    }
}