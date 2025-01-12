#include "MeloWebSocketService.h"

#include "../Utils/JuceLocalStorage.h"

MeloWebSocketService::MeloWebSocketService(const juce::String &wsRoute): webSocket(ix::WebSocket()), wsRoute(wsRoute) {
    juce::Logger::outputDebugString(juce::String::fromUTF8("WebSocket initialisé."));
    auto accessToken = JuceLocalStorage::getInstance().loadValue("access_token");
    if (accessToken.isNotEmpty()) {
        webSocket.setExtraHeaders({{"Authorization", "Bearer " + accessToken.toStdString()}});
    }

    webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
    switch (msg->type) {
        case ix::WebSocketMessageType::Message:
            juce::Logger::outputDebugString(juce::String::fromUTF8(("Message reçu : " + msg->str).c_str()));
            break;
        case ix::WebSocketMessageType::Open:
            juce::Logger::outputDebugString("Connexion WebSocket ouverte.");
            break;
        case ix::WebSocketMessageType::Close:
            juce::Logger::outputDebugString("Connexion WebSocket fermée.");
            break;
        case ix::WebSocketMessageType::Error:
            juce::Logger::outputDebugString(juce::String::fromUTF8(("Erreur WebSocket : " + msg->errorInfo.reason).c_str()));
            break;
        // Vous pouvez ajouter d'autres cas si nécessaire
        default:
            break;
    }
});
};

MeloWebSocketService::~MeloWebSocketService() {
    webSocket.stop();
}

void MeloWebSocketService::connectToServer() {
    auto url = juce::String(Constants::websocketUrl + wsRoute).toStdString();
    juce::Logger::outputDebugString("Connecting to WebSocket server at " + url);

    webSocket.setUrl(url);

    webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        juce::Logger::outputDebugString("Message reçu.");
        switch (msg->type) {
            case ix::WebSocketMessageType::Open:
                juce::Logger::outputDebugString("WebSocket ouvert : " + juce::String(msg->openInfo.uri));
                break;
            case ix::WebSocketMessageType::Message:
                juce::Logger::outputDebugString("Message reçu : " + juce::String(msg->str));
                break;
            case ix::WebSocketMessageType::Error:
                juce::Logger::outputDebugString("Erreur WebSocket : " + juce::String(msg->errorInfo.reason));
                break;
            case ix::WebSocketMessageType::Close:
                juce::Logger::outputDebugString("WebSocket fermé.");
                break;
            default:
                juce::Logger::outputDebugString("Autre événement WebSocket détecté.");
                break;
        }
    });

    webSocket.connect(5000);
    webSocket.start();
}

void MeloWebSocketService::sendMessage(const std::string &message, int retryCounter) {
    if (retryCounter > 3) {
        std::cerr << "Tentatives d'envoi de message épuisées." << std::endl;
        return;
    }
    switch (webSocket.getReadyState()) {
        case ix::ReadyState::Connecting:
            std::cerr << "WebSocket en cours de connexion." << std::endl;
            break;
        case ix::ReadyState::Open:
            webSocket.sendText(message);
            std::cout << "Message envoyé : " << message << std::endl;
            break;
        case ix::ReadyState::Closed:
            connectToServer();
            sendMessage(message, retryCounter + 1);
            std::cerr << "WebSocket non connecté." << std::endl;
            break;
        case ix::ReadyState::Closing:
            std::cerr << "WebSocket en cours de fermeture." << std::endl;
            break;
    }
}
