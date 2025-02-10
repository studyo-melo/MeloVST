#include "WebRTCReceiverConnexionHandler.h"
#include "../Common/EventManager.h"
#include "../ThirdParty/json.hpp"
#include "../AudioSettings.h"
#include "../Api/SocketRoutes.h"

WebRTCReceiverConnexionHandler::WebRTCReceiverConnexionHandler(const WsRoute wsRoute,
                                               const rtc::Description::Direction trackDirection): meloWebSocketService(
        WebSocketService(getWsRouteString(wsRoute))),
    trackDirection(trackDirection),
    reconnectTimer([this]() { attemptReconnect(); }) {
    EventManager::getInstance().addListener(this);
}

WebRTCReceiverConnexionHandler::~WebRTCReceiverConnexionHandler() {
    EventManager::getInstance().removeListener(this);
    if (peerConnection) {
        peerConnection->close();
    }
}

void WebRTCReceiverConnexionHandler::setupConnection() {
    rtc::InitLogger(rtc::LogLevel::Info);
    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    peerConnection = std::make_shared<rtc::PeerConnection>(config);

    peerConnection->onLocalDescription([this](const rtc::Description &sdp) {
        juce::Logger::outputDebugString("Local description set");
        if (!peerConnection->remoteDescription()) {
            juce::Logger::outputDebugString("No remote description. Waiting for answer.");
            sendOfferToRemote(sdp);
        }
    });

    peerConnection->onLocalCandidate([this](const rtc::Candidate &candidate) {
        // if (peerConnection->signalingState() == rtc::PeerConnection::SignalingState::Stable) {
        //     return;
        // }

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

    peerConnection->onTrack([this](const std::shared_ptr<rtc::Track> &track) {
        juce::Logger::outputDebugString("Track received");
        audioTrack = track;
        track->onFrame([this](const rtc::binary &data, const rtc::FrameInfo frameInfo) {
            EventManager::getInstance().notifyOnAudioBlockReceived({data, frameInfo});
        });
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
}

void WebRTCReceiverConnexionHandler::disconnect() const {
    if (peerConnection) {
        peerConnection->close();
    }
}

void WebRTCReceiverConnexionHandler::resetConnection() {
    disconnect();
    setupConnection();
}

void WebRTCReceiverConnexionHandler::notifyRTCStateChanged() const {
    juce::Logger::outputDebugString("RTC state changed, notifying listeners");
    EventManager::getInstance().notifyOnRTCStateChanged({
        peerConnection->state(), peerConnection->iceState(), peerConnection->signalingState()
    });
}


void WebRTCReceiverConnexionHandler::onWsMessageReceived(const MessageWsReceivedEvent &event) {
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

void WebRTCReceiverConnexionHandler::setOffer() {
    juce::Logger::outputDebugString("Setting offer");
    if (peerConnection->signalingState() != rtc::PeerConnection::SignalingState::Stable && peerConnection->state() ==
        rtc::PeerConnection::State::Connected) {
        return;
    }
    if (peerConnection->localDescription().has_value()) {
        juce::Logger::outputDebugString("Offer already set, sending it to remote");
        sendOfferToRemote(peerConnection->localDescription().value());
    } else {
        juce::Logger::outputDebugString("Creating new offer");
        peerConnection->localDescription().reset();
        peerConnection->setLocalDescription(rtc::Description::Type::Offer);
    }
}

void WebRTCReceiverConnexionHandler::handleAnswer(const std::string &sdp) {
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

void WebRTCReceiverConnexionHandler::onOngoingSessionChanged(const OngoingSessionChangedEvent &event) {
    ongoingSession = event.ongoingSession;
    meloWebSocketService.connectToServer();
}

void WebRTCReceiverConnexionHandler::attemptReconnect() {
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

void WebRTCReceiverConnexionHandler::monitorAnswer() {
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

bool WebRTCReceiverConnexionHandler::isConnected() const {
    if (!peerConnection) {
        return false;
    }
    return peerConnection->state() == rtc::PeerConnection::State::Connected;
}

bool WebRTCReceiverConnexionHandler::isConnecting() const {
    if (!peerConnection) {
        return false;
    }
    return peerConnection->state() == rtc::PeerConnection::State::Connecting;
}

void WebRTCReceiverConnexionHandler::sendOfferToRemote(const rtc::Description &sdp) {
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

void WebRTCReceiverConnexionHandler::sendCandidateToRemote(const rtc::Candidate &candidate) {
    if (!ongoingSession.has_value()) {
        return;
    }
    const auto candidateEvent = new RTCIceCandidateSentEvent(candidate, ongoingSession.value());
    meloWebSocketService.sendMessage(candidateEvent->createMessage());
}

juce::String WebRTCReceiverConnexionHandler::getSignalingStateLabel() const {
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

juce::String WebRTCReceiverConnexionHandler::getIceCandidateStateLabel() const {
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
