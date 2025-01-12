#include "MeloWebRTCServerService.h"

#include "../../../../../../Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX15.2.sdk/System/Library/Frameworks/Security.framework/Headers/SecCustomTransform.h"

MeloWebRTCServerService::MeloWebRTCServerService():
    meloWebSocketService(MeloWebSocketService(getWsRouteString(WsRoute::GetOngoingSessionRTC))),
    reconnectTimer([this]() { attemptReconnect(); })
{
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
    // rtc::Description::Audio newAudioTrack("audio", rtc::Description::Direction::SendOnly);
    // newAudioTrack.addOpusCodec(111);
    // newAudioTrack.addSSRC(12345678, "audioStream");
    // audioTrack = peerConnection->addTrack(static_cast<rtc::Description::Media>(newAudioTrack));

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

    peerConnection->onIceStateChange([this](rtc::PeerConnection::IceState state) {
        switch (state) {
            case rtc::PeerConnection::IceState::New:
                juce::Logger::outputDebugString("ICE state changed to: New");
                break;
            case rtc::PeerConnection::IceState::Disconnected:
            case rtc::PeerConnection::IceState::Failed:
            case rtc::PeerConnection::IceState::Closed:
                reconnectTimer.callAfterDelay(reconnectDelayMs, [this]() { attemptReconnect(); });
                break;
            case rtc::PeerConnection::IceState::Connected: {
                reconnectAttempts = 0;
                juce::Logger::outputDebugString("ICE state changed to: Connected");
                break;
            }

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
        rtc::binary binaryData = VectorUtils::convertFloatToBinary(channelData, event.buffer.getNumSamples());

        dataChannel->send(binaryData);
    }
};


void MeloWebRTCServerService::onWsMessageReceived(const MessageWsReceivedEvent &event) {
    if (event.type == "answer" && event.data.contains("answerSdp")) {
        handleAnswer(event.data["answerSdp"]);
    }

    else if (event.type == "candidate") {
        std::string candidate = event.data["candidate"];
        std::string sdpMid = event.data["sdpMid"];
        // int sdpMLineIndex = event.data["sdpMLineIndex"];

        rtc::Candidate iceCandidate(rtc::Candidate(candidate, sdpMid));
        peerConnection->addRemoteCandidate(iceCandidate);
    }
}


void MeloWebRTCServerService::attemptReconnect() {
    if (reconnectAttempts >= maxReconnectAttempts) {
        juce::Logger::outputDebugString("Max reconnect attempts reached. Giving up.");
        return;
    }

    juce::Logger::outputDebugString("Attempting to reconnect...");
    reconnectAttempts++;

    resetConnection();

    reconnectTimer.callAfterDelay(reconnectDelayMs, [this]() {
        if (!peerConnection || peerConnection->state() == rtc::PeerConnection::State::Closed) {
            attemptReconnect();
        } else {
            juce::Logger::outputDebugString("Reconnection successful.");
            reconnectAttempts = 0;
        }
    });
}

void MeloWebRTCServerService::resetConnection() {
    if (peerConnection) {
        peerConnection->close();
        peerConnection.reset();
    }

    setupConnection();
}