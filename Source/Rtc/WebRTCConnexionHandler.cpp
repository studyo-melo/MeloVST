#include "WebRTCConnexionHandler.h"

WebRTCConnexionHandler::WebRTCConnexionHandler(): meloWebSocketService(
                                                WebSocketService(
                                                    getWsRouteString(WsRoute::GetOngoingSessionRTC))),
                                            reconnectTimer([this]() { attemptReconnect(); })
{
    EventManager::getInstance().addListener(this);
}

WebRTCConnexionHandler::~WebRTCConnexionHandler() {
    EventManager::getInstance().removeListener(this);
    if (yangPeerConnection) {
        yangPeerConnection->close();
    }
}

void WebRTCConnexionHandler::setupConnection() {
    char* localSdp=NULL;
    char* remoteSdp=NULL;
    auto context =new YangContext();
    auto* streamConfig = new YangStreamConfig();
    yangPeerConnection = new YangPeerConnection2(&context->avinfo, streamConfig);


    yangPeerConnection->addTransceiver(YangSendonly);
    yangPeerConnection->addAudioTrack(Yang_AED_OPUS);

    yangPeerConnection->streamConfig->iceCallback.onIceStateChange = [](void* context, int32_t uid, YangIceCandidateType iceCandidateType, YangIceCandidateState iceState) {
        if (context) {
            static_cast<WebRTCConnexionHandler*>(context)->iceState = iceState;
            static_cast<WebRTCConnexionHandler*>(context)->notifyRTCStateChanged();
            static_cast<WebRTCConnexionHandler*>(context)->onIceStateChange(context, uid, iceCandidateType, iceState);
        }
    };

    yangPeerConnection->streamConfig->iceCallback.onConnectionStateChange = [](void* context, int32_t uid, YangRtcConnectionState connectionState) {
        if (context) {
            static_cast<WebRTCConnexionHandler*>(context)->connectionState = connectionState;
            static_cast<WebRTCConnexionHandler*>(context)->notifyRTCStateChanged();
            if (connectionState == Yang_Conn_State_Connected) {
                static_cast<WebRTCConnexionHandler*>(context)->monitorAnswer();
            }
        }
    };

    // yangPeerConnection->streamConfig->iceCallback

    auto offerOk = yangPeerConnection->createOffer(&localSdp);

    if(offerOk != Yang_Ok){
        yang_error("createOffer fail!");
    }

//    yangPeerConnection->connectSfuServer();
    sendOfferToRemote(localSdp);
    // peerConnection->onLocalDescription([this](rtc::Description sdp) {
    //     if (!peerConnection->remoteDescription()) {
    //         sendOfferToRemote(sdp);
    //     }
    // });

    // peerConnection->onLocalCandidate([this](const rtc::Candidate &candidate) {
    //     if (peerConnection->signalingState() == rtc::PeerConnection::SignalingState::Stable) {
    //         return;
    //     }
    //
    //     if (peerConnection->remoteDescription()) {
    //         sendCandidateToRemote(candidate);
    //     }
    //     else {
    //         pendingCandidates.push_back(candidate);
    //         juce::Logger::outputDebugString("Candidate stored temporarily. Waiting for remote description.");
    //     }
    // });
    //
    //
    // peerConnection->onTrack([this](std::shared_ptr<rtc::Track> track) {
    //     juce::Logger::outputDebugString("Track received");
    // });
    //
}

void WebRTCConnexionHandler::onIceStateChange(void* context,int32_t uid,YangIceCandidateType iceCandidateType,YangIceCandidateState iceState) {
    switch (iceState) {
        case YangIceNew:
            juce::Logger::outputDebugString("ICE state changed to: New");
        break;
        case YangIceFail: {
            juce::Logger::outputDebugString("ICE state changed to: Disconnected / Failed / Closed");
            reconnectTimer.callAfterDelay(reconnectDelayMs, [this]() { attemptReconnect(); });
            break;
        }
        case YangIceSuccess: {
            if (yangPeerConnection->isConnected()) {
                juce::Logger::outputDebugString("Already connected, skipping reconnection.");
            }
            reconnectAttempts = 0;
            juce::Logger::outputDebugString("ICE state changed to: Connected");
            break;
        }
        default:
            break;
    }
}

void WebRTCConnexionHandler::disconnect() const {
    if (yangPeerConnection) {
        yangPeerConnection->close();
    }
}

void WebRTCConnexionHandler::resetConnection() {
    disconnect();
    setupConnection();
}

void WebRTCConnexionHandler::notifyRTCStateChanged() const {
    juce::Logger::outputDebugString("RTC state changed, notifying listeners");
    EventManager::getInstance().notifyOnRTCStateChanged({connectionState, iceState, signalingState});
}


void WebRTCConnexionHandler::onWsMessageReceived(const MessageWsReceivedEvent &event) {
    if (!yangPeerConnection) {
        return;
    }

    if (event.type == "answer" && event.data.contains("answerSdp")) {
        std::string answerSdp = event.data["answerSdp"];
        yangPeerConnection->setRemoteDescription(answerSdp.data());
    } else if (event.type == "candidate") {
        if (yangPeerConnection->isConnected()) {
            return;
        }
        juce::Logger::outputDebugString("Received ICE candidate ->" + event.data["candidate"]);
        std::string candidate = event.data["candidate"];
        std::string sdpMid = event.data["sdpMid"];

        // rtc::Candidate iceCandidate(rtc::Candidate(candidate, sdpMid));
        // yangPeerConnection->streamConfig
        // peerConnection->addRemoteCandidate(iceCandidate);
    }
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
    if (yangPeerConnection && connectionState == Yang_Conn_State_Connected) {
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
        if (!yangPeerConnection || connectionState == Yang_Conn_State_Closed) {
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
    if (!yangPeerConnection) {
        return false;
    }
    return yangPeerConnection->isConnected();
}

bool WebRTCConnexionHandler::isConnecting() const {
    if (!yangPeerConnection) {
        return false;
    }
    return yangPeerConnection->isAlive();
}

void WebRTCConnexionHandler::sendOfferToRemote(char* sdp) {
    if (ongoingSession.has_value()) {
        const auto offerEvent = new RTCOfferSentEvent(sdp, ongoingSession.value());
        meloWebSocketService.sendMessage(offerEvent->createMessage());
        monitorAnswer();
    }
}

// void WebRTCConnexionHandler::sendCandidateToRemote(const rtc::Candidate &candidate) {
//     if (!ongoingSession.has_value()) {
//         return;
//     }
//     const auto candidateEvent = new RTCIceCandidateSentEvent(candidate, ongoingSession.value());
//     meloWebSocketService.sendMessage(candidateEvent->createMessage());
// }


