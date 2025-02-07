#pragma once
#include <functional>
#include <juce_core/juce_core.h>

class ReconnectTimer : public juce::Timer {
public:
    ReconnectTimer(std::function<void()> reconnectFunction)
        : reconnectFunction(reconnectFunction) {}

    void timerCallback() override {
        if (reconnectFunction) {
            reconnectFunction(); // Appeler la fonction de reconnexion
        }
    }

private:
    std::function<void()> reconnectFunction; // Fonction de rappel
};
