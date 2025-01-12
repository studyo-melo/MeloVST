#include "MeloWebRTCServerService.h"

#include "../../../../../../Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX15.2.sdk/System/Library/Frameworks/Security.framework/Headers/SecCustomTransform.h"

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
    rtc::Description::Audio newAudioTrack("audio", rtc::Description::Direction::SendOnly);
    newAudioTrack.addOpusCodec(111);
    newAudioTrack.addSSRC(12345678, "audioStream");
    audioTrack = peerConnection->addTrack(static_cast<rtc::Description::Media>(newAudioTrack));

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
    if (!dataChannel || !dataChannel->isOpen()) {
        return;
    }
    handleAudioData(event);
};

void MeloWebRTCServerService::handleAudioData(const AudioBlockProcessedEvent &event) const {
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
