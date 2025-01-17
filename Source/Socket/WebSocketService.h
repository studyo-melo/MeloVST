#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <ixwebsocket/ixWebsocket.h>
#include "SocketRoutes.h"
#include "../Utils/Constants.h"

class WebSocketService
{
public:
    WebSocketService(const juce::String& wsRoute);
    ~WebSocketService();

    void connectToServer();
    void disconnectToServer();
    void sendMessage(const std::string& message, int retryCounter = 0);

private:
    juce::String wsRoute;
    ix::WebSocket webSocket;
};
