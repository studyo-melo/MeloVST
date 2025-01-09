#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

class MeloWebSocketService
{
public:
    MeloWebSocketService();
    ~MeloWebSocketService();

    void connectToServer(const juce::String &route) const;
    void sendMessage(const juce::String& message) const;

private:
    std::unique_ptr<juce::StreamingSocket> socket;
};
