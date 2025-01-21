#pragma once
#include <juce_core/juce_core.h>

namespace FileUtils {
    static std::string generateTimestampedFilename(const std::string& baseName, const std::string& extension) {
    // Obtenir l'heure actuelle
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    auto localTime = *std::localtime(&timeT);

    // Formater l'horodatage
    std::ostringstream oss;
    oss << baseName << "_"
        << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S") // Format : Année-Mois-Jour_Heure-Minute-Seconde
        << "." << extension;

    return oss.str();
}
}