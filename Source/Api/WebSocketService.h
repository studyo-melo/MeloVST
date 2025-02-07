#pragma once

#include <juce_core/juce_core.h>
#include <ixwebsocket/IXWebSocket.h>

class WebSocketService
{
public:
    explicit WebSocketService(const juce::String& wsRoute);
    ~WebSocketService();

    void connectToServer();
    void disconnectToServer();
    void sendMessage(const std::string& message, int retryCounter = 0);

private:
    juce::String wsRoute;
    ix::WebSocket webSocket;
};
