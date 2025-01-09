#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace Constants
{
    // URL de l'API
    static const juce::String apiUrl = "https://staging.studio-melo.com/api";
    static const juce::String websocketUrl = "wss://staging.studio-melo.com";
//    static const juce::String apiUrl = "http://localhost:5055/api";
//    static const juce::String websocketUrl = "ws://localhost:8080";

    static const juce::String appName = "MeloVST";
    static constexpr int timeoutDuration = 3000; // en millisecondes
}
