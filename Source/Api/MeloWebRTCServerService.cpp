#include "MeloWebRTCServerService.h"

MeloWebRTCServerService::MeloWebRTCServerService(): meloWebSocketService(MeloWebSocketService(getWsRouteString(WsRoute::GetOngoingSessionRTC))) {
    EventManager::getInstance().addListener(this);
}
MeloWebRTCServerService::~MeloWebRTCServerService() {
    EventManager::getInstance().removeListener(this);
    if (peerConnection) {
        peerConnection->close();
    }
}

void MeloWebRTCServerService::setupConnection() {
    if (peerConnection) {
        peerConnection->close();
    }
    rtc::InitLogger(rtc::LogLevel::Info);
    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    peerConnection = std::make_shared<rtc::PeerConnection>(config);
    peerConnection->onLocalDescription([this](rtc::Description sdp) {
        if (ongoingSession.has_value()) {
            const auto offerEvent = new RTCOfferSentEvent(sdp, ongoingSession.value());
            meloWebSocketService.sendMessage(offerEvent->createMessage());
        }
    });

    peerConnection->onLocalCandidate([this](const rtc::Candidate &candidate) {
        if (ongoingSession.has_value()) {
            const auto candidateEvent = new RTCIceCandidateSentEvent(candidate, ongoingSession.value());
            meloWebSocketService.sendMessage(candidateEvent->createMessage());
        }
    });

    dataChannel = peerConnection->createDataChannel("secure-audio");
    // peerConnection->setLocalDescription(rtc::Description::Type::Offer);

    // Configurez les callbacks
    // peerConnection->onTrack([this](std::shared_ptr<rtc::Track> track) {
    //     std::cout << "Track received: " << track << std::endl;
    //
    //     // Gérer l'audio ici (par ex., streamer vers l'application React)
    // });

    // peerConnection->onDataChannel([this](std::shared_ptr<rtc::DataChannel> channel) {
    //     std::cout << "Data channel opened: " << channel->label() << std::endl;
    //     dataChannel = channel;
    //
    //     // Gérer les messages reçus (par exemple, commandes ou contrôle)
    //     dataChannel->onMessage(
    //     [](const rtc::binary& data) {
    //         std::cout << "Received binary data: " << data.size() << " bytes" << std::endl;
    //     },
    //     [](const std::string& data) {
    //         std::cout << "Received text data: " << data << std::endl;
    //     });
    // });
    // Créer un canal de données pour transmettre l'audio

}

void MeloWebRTCServerService::setOffer() const {
    peerConnection->setLocalDescription(rtc::Description::Type::Offer);
}
void MeloWebRTCServerService::handleAnswer(const std::string& sdp) const {
    // Réception de la réponse SDP depuis le client React
    peerConnection->setRemoteDescription(rtc::Description(sdp, "answer"));
}

void MeloWebRTCServerService::onOngoingSessionChanged(const OngoingSessionChangedEvent& event) {
    this->ongoingSession = event.ongoingSession;
    meloWebSocketService.connectToServer();
    this->setupConnection();
};

void MeloWebRTCServerService::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent& event) {
    if (!dataChannel) {
        return;
    }
    dataChannel->onOpen([event, this]() {
        for (int channel = 0; channel < event.totalNumInputChannels; ++channel)
        {
            auto* channelData = event.buffer.getWritePointer (channel);
            if (!dataChannel || !dataChannel->isOpen()) {
            std::cerr << "Data channel is not open!" << std::endl;
            }
            juce::Logger::outputDebugString("Sending audio data");
            // Convertir les données audio en format binaire
            rtc::binary binaryData = VectorUtils::convertFloatToBinary(channelData, event.buffer.getNumSamples());

            // Envoyer les données via le canal de données
            dataChannel->send(binaryData);
        }
    });

};


void MeloWebRTCServerService::onWsMessageReceived(const MessageWsReceivedEvent &event) {
    if (event.type == "answer" && event.data.contains("answerSdp")) {
        handleAnswer(event.data["answerSdp"]);
    }

    else if (event.type == "candidate") {
        // Gérer le candidat ICE reçu du WebSocket
        std::string candidate = event.data["candidate"];
        std::string sdpMid = event.data["sdpMid"];
        // int sdpMLineIndex = event.data["sdpMLineIndex"];

        // Créer le candidat ICE
        rtc::Candidate iceCandidate(rtc::Candidate(candidate, sdpMid));

        // Ajouter le candidat à la connexion
        peerConnection->addRemoteCandidate(iceCandidate);
    }
}
