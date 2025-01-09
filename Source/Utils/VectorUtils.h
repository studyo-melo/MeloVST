#pragma once
#include <vector>
#include <rtc/rtc.hpp>

namespace VectorUtils {
    static rtc::binary convertFloatToBinary(const float* data, size_t size) {
        // Allouer suffisamment d'espace pour contenir les données binaires
        rtc::binary binaryData(size * sizeof(float));

        // Copier les données brutes dans le tableau binaire
        std::memcpy(binaryData.data(), data, size * sizeof(float));

        return binaryData;
    }
}
