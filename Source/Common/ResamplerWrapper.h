//
// Created by Padoa on 27/01/2025.
//

#pragma once
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include <CDSPResampler.h>

class ResamplerWrapper {
public:
    ResamplerWrapper(double sourceSR, double targetSR, const int nChan) {
        resampler = std::make_unique<r8b::CDSPResampler>(sourceSR, targetSR, 512);
        sourceSampleRate = sourceSR;
        targetSampleRate = targetSR;
        numChannels = nChan;
    };

    std::vector<float> resampleFromFloat(const std::vector<float>& sourceVector) const {
        std::vector<double> doubleAudioBlock(sourceVector.begin(), sourceVector.end());
        std::vector<double> resampledDoubleAudioBlock = resampleFromDouble(doubleAudioBlock);

        std::vector<float> resampledAudioBlock(resampledDoubleAudioBlock.begin(), resampledDoubleAudioBlock.end());
        return resampledAudioBlock;
    }

    std::vector<double> resampleFromDouble(std::vector<double>& sourceVector) const {
        std::vector<double> outVector(std::floor(sourceVector.size() * targetSampleRate / sourceSampleRate), 0.0);
        double* outBuffer = outVector.data();
        resampler->process(sourceVector.data(), sourceVector.size(), outBuffer);
        for (auto i = 0; i < outVector.size(); ++i) {
            outVector[i] = std::clamp(outBuffer[i], -1.0, 1.0);
        }
        return outVector;
    }
    std::vector<int16_t> resampleFromInt16(const std::vector<int16_t>& sourceVector) const {
        std::vector<double> doubleAudioBlock(sourceVector.size());
        for (auto i = 0; i < sourceVector.size(); ++i) {
            doubleAudioBlock[i] = static_cast<double>(sourceVector[i] / 32768.0);
        }

        const std::vector<double> resampledDoubleAudioBlock = resampleFromDouble(doubleAudioBlock);

        std::vector<int16_t> resampledAudioBlock(resampledDoubleAudioBlock.size());
        for (auto i = 0; i < resampledDoubleAudioBlock.size(); ++i) {
            resampledAudioBlock[i] = static_cast<int16_t>(resampledDoubleAudioBlock[i] * 32768.0);
        }

        return resampledAudioBlock;
    }

private:
    std::unique_ptr<r8b::CDSPResampler> resampler;
    double sourceSampleRate;
    double targetSampleRate;
    int numChannels;

};
