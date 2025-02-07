#include "WebRTCConnexionHandler.h"
#include "../Common/EventManager.h"
#include "../ThirdParty/json.hpp"
#include "../AudioSettings.h"

WebRTCConnexionHandler::WebRTCConnexionHandler(const WsRoute wsRoute, const rtc::Description::Direction trackDirection):
    meloWebSocketService(WebSocketService(getWsRouteString(wsRoute))),
    trackDirection(trackDirection),
    reconnectTimer([this]() { attemptReconnect(); })
{
    EventManager::getInstance().addListener(this);
}

WebRTCConnexionHandler::~WebRTCConnexionHandler() {
    EventManager::getInstance().removeListener(this);
    if (peerConnection) {
        peerConnection->close();
    }
}

void WebRTCConnexionHandler::setupConnection() {
    rtc::InitLogger(rtc::LogLevel::Info);
    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    peerConnection = std::make_shared<rtc::PeerConnection>(config);

    rtc::Description::Audio newAudioTrack{};
    newAudioTrack.addOpusCodec(111, "minptime=10;useinbandfec=0");
    newAudioTrack.setBitrate(AudioSettings::getInstance().getOpusBitRate()); // Débit binaire en bits par seconde
    newAudioTrack.setDirection(trackDirection);
    newAudioTrack.addSSRC(12345, "CNAME");;

    peerConnection->onLocalDescription([this](const rtc::Description &sdp) {
        if (!peerConnection->remoteDescription()) {
            sendOfferToRemote(sdp);
        }
    });

    peerConnection->onLocalCandidate([this](const rtc::Candidate &candidate) {
        if (peerConnection->signalingState() == rtc::PeerConnection::SignalingState::Stable) {
            return;
        }

        if (peerConnection->remoteDescription()) {
            sendCandidateToRemote(candidate);
        } else {
            pendingCandidates.push_back(candidate);
            juce::Logger::outputDebugString("Candidate stored temporarily. Waiting for remote description.");
        }
    });

    peerConnection->onStateChange([this](rtc::PeerConnection::State state) {
        notifyRTCStateChanged();
    });

    peerConnection->onSignalingStateChange([this](rtc::PeerConnection::SignalingState state) {
        notifyRTCStateChanged();
    });

    peerConnection->onTrack([this](std::shared_ptr<rtc::Track> track) {
        juce::ignoreUnused(track);
        juce::Logger::outputDebugString("Track received");
    });

    peerConnection->onIceStateChange([this](const rtc::PeerConnection::IceState state) {
        notifyRTCStateChanged();
        switch (state) {
            case rtc::PeerConnection::IceState::New:
                juce::Logger::outputDebugString("ICE state changed to: New");
                break;
            case rtc::PeerConnection::IceState::Closed:
            case rtc::PeerConnection::IceState::Disconnected: {
                break;
            }
            case rtc::PeerConnection::IceState::Failed: {
                juce::Logger::outputDebugString("ICE state changed to: Disconnected / Failed / Closed");
                reconnectTimer.callAfterDelay(reconnectDelayMs, [this]() { attemptReconnect(); });
                break;
            }
            case rtc::PeerConnection::IceState::Checking:
                break;
            case rtc::PeerConnection::IceState::Completed:
            case rtc::PeerConnection::IceState::Connected: {
                if (peerConnection->state() == rtc::PeerConnection::State::Connected) {
                    juce::Logger::outputDebugString("Already connected, skipping reconnection.");
                    return;
                }
                reconnectAttempts = 0;
                juce::Logger::outputDebugString("ICE state changed to: Connected");
                break;
            }
            default:
                break;
        }
    });

    audioTrack = peerConnection->addTrack(static_cast<rtc::Description::Media>(newAudioTrack));
    setOffer();
}

void WebRTCConnexionHandler::disconnect() const {
    if (peerConnection) {
        peerConnection->close();
    }
}

void WebRTCConnexionHandler::resetConnection() {
    disconnect();
    setupConnection();
}

void WebRTCConnexionHandler::notifyRTCStateChanged() const {
    juce::Logger::outputDebugString("RTC state changed, notifying listeners");
    EventManager::getInstance().notifyOnRTCStateChanged({
        peerConnection->state(), peerConnection->iceState(), peerConnection->signalingState()
    });
}


void WebRTCConnexionHandler::onWsMessageReceived(const MessageWsReceivedEvent &event) {
    if (!peerConnection) {
        return;
    }
    if (peerConnection->signalingState() != rtc::PeerConnection::SignalingState::HaveLocalOffer) {
        return;
    }

    if (event.type == "answer" && event.data.contains("answerSdp")) {
        handleAnswer(event.data["answerSdp"]);
    } else if (event.type == "candidate") {
        if (peerConnection->state() == rtc::PeerConnection::State::Connected) {
            return;
        }
        juce::Logger::outputDebugString("Received ICE candidate ->" + event.data["candidate"]);
        const std::string candidate = event.data["candidate"];
        const std::string sdpMid = event.data["sdpMid"];

        const auto iceCandidate(rtc::Candidate(candidate, sdpMid));
        peerConnection->addRemoteCandidate(iceCandidate);
    }
}

void WebRTCConnexionHandler::setOffer() {
    if (peerConnection->signalingState() != rtc::PeerConnection::SignalingState::Stable && peerConnection->state() ==
        rtc::PeerConnection::State::Connected) {
        return;
    }
    if (peerConnection->localDescription().has_value()) {
        sendOfferToRemote(peerConnection->localDescription().value());
    } else {
        peerConnection->localDescription().reset();
        peerConnection->setLocalDescription(rtc::Description::Type::Offer);
    }
}

void WebRTCConnexionHandler::handleAnswer(const std::string &sdp) {
    if (peerConnection->signalingState() != rtc::PeerConnection::SignalingState::HaveLocalOffer) {
        return;
    }
    if (peerConnection->state() == rtc::PeerConnection::State::Connected) {
        return;
    }

    if (peerConnection->remoteDescription()) {
        return;
    }

    juce::Logger::outputDebugString("Received answer");
    peerConnection->setRemoteDescription(rtc::Description(sdp, "answer"));
    answerReceived = true;
    for (const auto &candidate: pendingCandidates) {
        sendCandidateToRemote(candidate);
    }
    pendingCandidates.clear();
}

void WebRTCConnexionHandler::onOngoingSessionChanged(const OngoingSessionChangedEvent &event) {
    ongoingSession = event.ongoingSession;
    meloWebSocketService.connectToServer();
}

void WebRTCConnexionHandler::attemptReconnect() {
    if (!ongoingSession.has_value()) {
        juce::Logger::outputDebugString("No ongoing session. Cannot reconnect.");
        return;
    }
    if (peerConnection && peerConnection->state() == rtc::PeerConnection::State::Connected) {
        juce::Logger::outputDebugString("Already connected. Skipping reconnection.");
        return;
    }
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

void WebRTCConnexionHandler::monitorAnswer() {
    if (!answerTimer.has_value()) {
        answerTimer.emplace(ReconnectTimer([this]() {
            if (answerReceived) {
                juce::Logger::outputDebugString("Answer received. Stopping the timer.");
                answerTimer->stopTimer();
                return;
            }

            if (resendAttempts >= maxResendAttempts) {
                juce::Logger::outputDebugString("Max resend attempts reached. Stopping resends.");
                answerTimer->stopTimer(); // Arrêter le timer après trop de tentatives
                return;
            }

            juce::Logger::outputDebugString("Resending offer...");
            resetConnection();
            resendAttempts++;
        }));
    }
    answerReceived = false;
    resendAttempts = 0;

    answerTimer->startTimer(resendIntervalMs);
}

bool WebRTCConnexionHandler::isConnected() const {
    if (!peerConnection) {
        return false;
    }
    return peerConnection->state() == rtc::PeerConnection::State::Connected;
}

bool WebRTCConnexionHandler::isConnecting() const {
    if (!peerConnection) {
        return false;
    }
    return peerConnection->state() == rtc::PeerConnection::State::Connecting;
}

void WebRTCConnexionHandler::sendOfferToRemote(const rtc::Description &sdp) {
    if (peerConnection->signalingState() != rtc::PeerConnection::SignalingState::HaveLocalOffer
        || peerConnection->state() == rtc::PeerConnection::State::Connected) {
        return;
    }
    if (ongoingSession.has_value()) {
        const auto offerEvent = new RTCOfferSentEvent(sdp, ongoingSession.value());
        meloWebSocketService.sendMessage(offerEvent->createMessage());
        monitorAnswer();
    }
}

void WebRTCConnexionHandler::sendCandidateToRemote(const rtc::Candidate &candidate) {
    if (!ongoingSession.has_value()) {
        return;
    }
    const auto candidateEvent = new RTCIceCandidateSentEvent(candidate, ongoingSession.value());
    meloWebSocketService.sendMessage(candidateEvent->createMessage());
}

juce::String WebRTCConnexionHandler::getSignalingStateLabel() const {
    if (!peerConnection) {
        return juce::String::fromUTF8("Inconnu");
    }
    switch (peerConnection->signalingState()) {
        case rtc::PeerConnection::SignalingState::Stable:
            return juce::String::fromUTF8("Stable");
        case rtc::PeerConnection::SignalingState::HaveLocalOffer:
            return juce::String::fromUTF8("Offre locale");
        case rtc::PeerConnection::SignalingState::HaveRemoteOffer:
            return juce::String::fromUTF8("Offre distante");
        case rtc::PeerConnection::SignalingState::HaveLocalPranswer:
            return juce::String::fromUTF8("Pré-réponse locale");
        case rtc::PeerConnection::SignalingState::HaveRemotePranswer:
            return juce::String::fromUTF8("Pré-réponse distante");
        default:
            return juce::String::fromUTF8("Inconnu");
    }
}

juce::String WebRTCConnexionHandler::getIceCandidateStateLabel() const {
    if (!peerConnection) {
        return juce::String::fromUTF8("Inconnu");
    }
    switch (peerConnection->iceState()) {
        case rtc::PeerConnection::IceState::New:
            return juce::String::fromUTF8("Nouveau");
        case rtc::PeerConnection::IceState::Checking:
            return juce::String::fromUTF8("Vérification");
        case rtc::PeerConnection::IceState::Connected:
            return juce::String::fromUTF8("Connecté");
        case rtc::PeerConnection::IceState::Completed:
            return juce::String::fromUTF8("Complété");
        case rtc::PeerConnection::IceState::Failed:
            return juce::String::fromUTF8("Échoué");
        case rtc::PeerConnection::IceState::Disconnected:
            return juce::String::fromUTF8("Déconnecté");
        case rtc::PeerConnection::IceState::Closed:
            return juce::String::fromUTF8("Fermé");
        default:
            return juce::String::fromUTF8("Inconnu");
    }
}
