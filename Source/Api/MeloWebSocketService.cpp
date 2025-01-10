#include "MeloWebSocketService.h"

MeloWebSocketService::MeloWebSocketService(): webSocket(std::make_shared<ix::WebSocket>().get()) {
    // Callback pour les messages reçus
    webSocket->setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::cout << "Message reçu : " << msg->str << std::endl;
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            std::cout << "Connexion WebSocket ouverte." << std::endl;
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            std::cout << "Connexion WebSocket fermée." << std::endl;
        }
    });
};

MeloWebSocketService::~MeloWebSocketService()
{
    webSocket->stop();
}

void MeloWebSocketService::connectToServer(const juce::String& wsRoute) {
    webSocket->setUrl(juce::String(Constants::websocketUrl + wsRoute).toStdString());
    webSocket->connect(5000);
}

void MeloWebSocketService::sendMessage(const std::string& message) {
    if (webSocket->getReadyState() == ix::ReadyState::Open) {
        webSocket->send(message);
        std::cout << "Message envoyé : " << message << std::endl;
    } else {
        std::cerr << "WebSocket non connecté." << std::endl;
    }
}
