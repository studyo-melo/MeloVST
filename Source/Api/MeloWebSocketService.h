#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <ixwebsocket/ixWebsocket.h>
#include "ApiRoutes.h"
#include "../Utils/Constants.h"

class MeloWebSocketService
{
public:
    MeloWebSocketService();
    ~MeloWebSocketService();

    void connectToServer(const juce::String& wsRoute);
    void sendMessage(const std::string& message);

private:
    ix::WebSocket webSocket;
};
