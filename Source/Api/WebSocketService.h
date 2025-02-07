#pragma once
#include "SocketRoutes.h"
#include "../Config.h"
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <ixwebsocket/IXWebSocket.h>
#include "../Common/EventManager.h"
#include "../Common/JuceLocalStorage.h"

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
