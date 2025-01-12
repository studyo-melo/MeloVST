#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <ixwebsocket/ixWebsocket.h>
#include "ApiRoutes.h"
#include "../Utils/Constants.h"

class MeloWebSocketService
{
public:
    MeloWebSocketService(const juce::String& wsRoute);
    ~MeloWebSocketService();

    void connectToServer();
    void disconnectToServer();
    void sendMessage(const std::string& message, int retryCounter = 0);

private:
    juce::String wsRoute;
    ix::WebSocket webSocket;
};
