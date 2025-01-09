#include <juce_gui_basics/juce_gui_basics.h>
#include "MeloWebSocketService.h"

#include "ApiRoutes.h"

MeloWebSocketService::MeloWebSocketService(): socket(new juce::StreamingSocket())
{
}

MeloWebSocketService::~MeloWebSocketService()
{
    if (socket != nullptr)
    {
        socket.release();
    }
}

// Méthode pour se connecter au serveur WebSocket
void MeloWebSocketService::connectToServer(const juce::String &route) const {
    if (socket != nullptr)
    {
        socket->close();
    }
    socket->connect(Constants::websocketUrl + route, Constants::websocketPort, 5000);
    socket->waitUntilReady(true, 5000);
    if (socket->isConnected())
    {
        juce::Logger::outputDebugString("Connexion WebSocket établie !");
    }
    else
    {
        juce::Logger::outputDebugString("Impossible de se connecter au WebSocket.");
    }
}

// Méthode pour envoyer un message sur le WebSocket
void MeloWebSocketService::sendMessage(const juce::String& message) const {
    if (socket != nullptr && socket->isConnected())
    {
        //message to buffer
        auto buffer = message.toRawUTF8();
        socket->write(buffer, std::strlen(buffer));
    }
}
