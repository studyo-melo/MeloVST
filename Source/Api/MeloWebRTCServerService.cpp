#include "MeloWebRTCServerService.h"

MeloWebRTCServerService::MeloWebRTCServerService(): meloWebSocketService(
                                                        MeloWebSocketService(
                                                            getWsRouteString(WsRoute::GetOngoingSessionRTC))),
                                                    stopThread(false),
                                                    reconnectTimer([this]() { attemptReconnect(); }) {
    EventManager::getInstance().addListener(this);
}

MeloWebRTCServerService::~MeloWebRTCServerService() {
    EventManager::getInstance().removeListener(this);
    stopAudioThread();
    if (peerConnection) {
        peerConnection->close();
    }
}

void MeloWebRTCServerService::pushAudioBuffer(const juce::AudioBuffer<float> &buffer) {
    // Convertit le buffer audio de JUCE en PCM 16 bits mono/stéréo
    std::vector<int16_t> pcmData(buffer.getNumSamples() * buffer.getNumChannels());
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const float *channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            int16_t pcmSample = static_cast<int16_t>(juce::jlimit<float>(-1.0f, 1.0f, channelData[sample]) * 32767);
            pcmData[sample * buffer.getNumChannels() + channel] = pcmSample;
        }
    }

    // Ajouter les données PCM dans la queue thread-safe
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        audioQueue.push(std::move(pcmData));
    }
    queueCondition.notify_one();
}

void MeloWebRTCServerService::handleAudioData(const AudioBlockProcessedEvent &event) {
    pushAudioBuffer(event.buffer);
};

void MeloWebRTCServerService::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    if (!audioTrack || !audioTrack->isOpen()) {
        return;
    }
    handleAudioData(event);
};

void MeloWebRTCServerService::setupConnection() {
    rtc::InitLogger(rtc::LogLevel::Info);
    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    peerConnection = std::make_shared<rtc::PeerConnection>(config);

    rtc::Description::Audio newAudioTrack{};
    newAudioTrack.addOpusCodec(111);
    newAudioTrack.setBitrate(64000);
    newAudioTrack.setDirection(rtc::Description::Direction::SendOnly);

    peerConnection->onLocalDescription([this](rtc::Description sdp) {
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
        }
        else {
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

    peerConnection->onIceStateChange([this](rtc::PeerConnection::IceState state) {
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
            case rtc::PeerConnection::IceState::Connected: {
                if (peerConnection->state() == rtc::PeerConnection::State::Connected) {
                    juce::Logger::outputDebugString("Already connected, skipping reconnection.");
                    return;
                }
                reconnectAttempts = 0;
                juce::Logger::outputDebugString("ICE state changed to: Connected");
                startAudioThread();
                break;
            }
            default:
                break;
        }
    });

    audioTrack = peerConnection->addTrack(static_cast<rtc::Description::Media>(newAudioTrack));
    setOffer();
}

void MeloWebRTCServerService::disconnect() {
    if (peerConnection) {
        peerConnection->close();
        peerConnection.reset();
        // stopThread = false;
    }
}

void MeloWebRTCServerService::resetConnection() {
    disconnect();
    setupConnection();
}

void MeloWebRTCServerService::startAudioThread() {
    juce::Logger::outputDebugString("Starting audio thread");
    stopAudioThread();
    stopThread = false;
    audioThread = std::thread([this]() {
        while (!stopThread) {
            std::vector<int16_t> pcmData; {
                std::unique_lock<std::mutex> lock(queueMutex);
                queueCondition.wait(lock, [this]() { return !audioQueue.empty() || stopThread; });
                if (stopThread) break;

                pcmData = std::move(audioQueue.front());
                audioQueue.pop();
            }

            juce::Logger::outputDebugString("Sending audio data");
            if (audioTrack) {
                try {
                    audioTrack->send(reinterpret_cast<const std::byte *>(pcmData.data()), pcmData.size() * sizeof(int16_t));
                }
                catch (const std::exception& e) {
                    juce::Logger::outputDebugString("Error sending audio data" + std::string(e.what()));
                }
            }
        }
    });
}

void MeloWebRTCServerService::stopAudioThread() {
    juce::Logger::outputDebugString("Stopping audio thread"); {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopThread = true;
    }
    queueCondition.notify_all();
    if (audioThread.joinable()) {
        audioThread.join();
    }
}

void MeloWebRTCServerService::notifyRTCStateChanged() const {
    juce::Logger::outputDebugString("RTC state changed, notifying listeners");
    EventManager::getInstance().notifyOnRTCStateChanged({
        peerConnection->state(), peerConnection->iceState(), peerConnection->signalingState()
    });
};


void MeloWebRTCServerService::onWsMessageReceived(const MessageWsReceivedEvent &event) {
    if (peerConnection->signalingState() != rtc::PeerConnection::SignalingState::HaveLocalOffer) {
        return;
    }

    if (event.type == "answer" && event.data.contains("answerSdp")) {
        handleAnswer(event.data["answerSdp"]);
    } else if (event.type == "candidate") {
        if (peerConnection->state() == rtc::PeerConnection::State::Connected) {
            return;
        }
        juce::Logger::outputDebugString("Received ICE candidate");
        std::string candidate = event.data["candidate"];
        std::string sdpMid = event.data["sdpMid"];

        rtc::Candidate iceCandidate(rtc::Candidate(candidate, sdpMid));
        peerConnection->addRemoteCandidate(iceCandidate);
    }
}

void MeloWebRTCServerService::setOffer() {
    if (peerConnection->signalingState() != rtc::PeerConnection::SignalingState::Stable && peerConnection->state() ==
        rtc::PeerConnection::State::Connected) {
        return;
    }
    if (peerConnection->localDescription().has_value()) {
        sendOfferToRemote(peerConnection->localDescription().value());
    }
    else {
        peerConnection->setLocalDescription(rtc::Description::Type::Offer);
    }
}

void MeloWebRTCServerService::handleAnswer(const std::string &sdp) {
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
    for (const auto &candidate : pendingCandidates) {
        sendCandidateToRemote(candidate);
    }
    pendingCandidates.clear();
}

void MeloWebRTCServerService::onOngoingSessionChanged(const OngoingSessionChangedEvent &event) {
    ongoingSession = event.ongoingSession;
    meloWebSocketService.connectToServer();
}

void MeloWebRTCServerService::attemptReconnect() {
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

void MeloWebRTCServerService::monitorAnswer() {
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

bool MeloWebRTCServerService::isConnected() const {
    if (!peerConnection) {
        return false;
    }
    return peerConnection->state() == rtc::PeerConnection::State::Connected;
}

void MeloWebRTCServerService::sendOfferToRemote(const rtc::Description &sdp) {
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

void MeloWebRTCServerService::sendCandidateToRemote(const rtc::Candidate &candidate) {
    if (!ongoingSession.has_value()) {
        return;
    }
    const auto candidateEvent = new RTCIceCandidateSentEvent(candidate, ongoingSession.value());
    meloWebSocketService.sendMessage(candidateEvent->createMessage());
}

juce::String MeloWebRTCServerService::getSignalingStateLabel() const {
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

juce::String MeloWebRTCServerService::getIceCandidateStateLabel() const {
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
