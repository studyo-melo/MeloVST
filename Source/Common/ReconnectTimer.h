#pragma once
#include <functional>

class ReconnectTimer : public juce::Timer {
public:
    ReconnectTimer(const std::function<void()> &reconnectFunction)
        : reconnectFunction(reconnectFunction) {}

    void timerCallback() override {
        if (reconnectFunction) {
            reconnectFunction(); // Appeler la fonction de reconnexion
        }
    }

private:
    std::function<void()> reconnectFunction; // Fonction de rappel
};
