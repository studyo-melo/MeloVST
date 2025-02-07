#pragma once
#include <vector>
#include <rtc/rtc.hpp>
#include <juce_core/juce_core.h>

namespace VectorUtils {
    static rtc::binary convertFloatToBinary(const float* data, const size_t size) {
        // Allouer suffisamment d'espace pour contenir les données binaires
        rtc::binary binaryData(size * sizeof(float));

        // Copier les données brutes dans le tableau binaire
        std::memcpy(binaryData.data(), data, size * sizeof(float));

        return binaryData;
    }

    static std::vector<float> convertInt16ToFloat(const int16_t* data, const size_t size) {
        // Allouer suffisamment d'espace pour contenir les données flottantes
        std::vector<float> floatData(0);
        floatData.resize(size);

        // Convertir les données int16_t en données flottantes
        for (size_t i = 0; i < size; ++i) {
            floatData[i] = static_cast<float>(data[i]) / static_cast<float>(INT16_MAX);
        }

        return floatData;
    }

    static std::vector<int16_t> convertFloatToInt16(const float* data, const size_t size) {
        // Allouer suffisamment d'espace pour contenir les données int16_t
        std::vector<int16_t> int16Data(size);

        // Convertir les données flottantes en données int16_t
        for (size_t i = 0; i < size; ++i) {
            int16Data[i] = static_cast<int16_t>(data[i] * INT16_MAX);
        }

        return int16Data;
    }

    static std::vector<int16_t> convertCharToInt16(const unsigned char* data, const size_t size) {
        std::vector<int16_t> int16Data(size / 2);
        for (size_t i = 0; i < size; i += 2) {
            int16Data[i / 2] = static_cast<int16_t>(data[i] | (data[i + 1] << 8));
        }
        return int16Data;
    }
}
