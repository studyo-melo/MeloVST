//
// Created by Padoa on 27/01/2025.
//

#pragma once
#include <CDSPResampler.h>

class ResamplerWrapper {
public:
    ResamplerWrapper(double sourceSR, double targetSR, int nChan) {
        resampler = std::make_unique<r8b::CDSPResampler>(sourceSampleRate, targetSampleRate, 512);
        sourceSampleRate = sourceSR;
        targetSampleRate = targetSR;
        targetSampleRate = nChan;
    };

    std::vector<int16_t> resampleFromInt16(const std::vector<int16_t>& sourceVector) const {
        int sourceLength = static_cast<int>(sourceVector.size()) / numChannels;
        int destLength = static_cast<int>(std::floor(sourceLength * targetSampleRate / sourceSampleRate));

        std::vector<int16_t> outVector(destLength * numChannels);
        for (int j = 0; j < numChannels; j++)
        {
            std::vector<double> sourceBuffer(sourceLength);
            for (int i = 0; i < sourceLength; i++)
            {
            sourceBuffer[i] = static_cast<double>(sourceVector[i * numChannels + j]) / 32768.0; // Normalisation
            }

            // Prepare output buffer
            std::vector<double> outBuffer(destLength);
            double* outBufferPtr = outBuffer.data();
            resampler->process(sourceBuffer.data(), sourceLength, outBufferPtr);
            for (int i = 0; i < destLength; i++)
            {
                outVector[i * numChannels + j] = static_cast<int16_t>(std::clamp(outBuffer[i] * 32768.0, -32768.0, 32767.0)); // Normalisation et conversion
            }
        }

        return outVector; // Return the resampled vector
    }

private:
    std::unique_ptr<r8b::CDSPResampler> resampler;
    double sourceSampleRate;
    double targetSampleRate;
    int numChannels;

};