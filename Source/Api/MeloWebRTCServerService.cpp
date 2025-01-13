#include "MeloWebRTCServerService.h"

#include "../../../../../../Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX15.2.sdk/System/Library/Frameworks/Security.framework/Headers/SecCustomTransform.h"

MeloWebRTCServerService::MeloWebRTCServerService():
meloWebSocketService(MeloWebSocketService(getWsRouteString(WsRoute::GetOngoingSessionRTC))),
reconnectTimer([this]() { attemptReconnect(); }),
stopThread(false)
{
    EventManager::getInstance().addListener(this);
}

MeloWebRTCServerService::~MeloWebRTCServerService() {
    EventManager::getInstance().removeListener(this);
    stopAudioThread();
    if (peerConnection) {
        peerConnection->close();
    }
}

void MeloWebRTCServerService::pushAudioBuffer(const juce::AudioBuffer<float>& buffer) {
    // Convertit le buffer audio de JUCE en PCM 16 bits mono/stéréo
    std::vector<int16_t> pcmData(buffer.getNumSamples() * buffer.getNumChannels());
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const float* channelData = buffer.getReadPointer(channel);
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
    auto numSamples = event.buffer.getNumSamples();
    auto* leftChannel = event.buffer.getReadPointer(0);
    auto* rightChannel = event.buffer.getReadPointer(1);

    // Convertir les échantillons en PCM 16 bits (format attendu par WebRTC)
    std::vector<int16_t> pcmData(numSamples * 2); // Stéréo
    for (int i = 0; i < numSamples; ++i) {
        pcmData[2 * i] = static_cast<int16_t>(leftChannel[i] * 32767.0f); // Canal gauche
        pcmData[2 * i + 1] = static_cast<int16_t>(rightChannel[i] * 32767.0f); // Canal droit
    }

    pushAudioBuffer(event.buffer);
};


void MeloWebRTCServerService::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    if (!audioTrack || !audioTrack->isOpen()) {
        return;
    }
    handleAudioData(event);
};

void MeloWebRTCServerService::setupConnection() {
    if (peerConnection) {
        peerConnection->close();
    }
    rtc::InitLogger(rtc::LogLevel::Info);
    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    peerConnection = std::make_shared<rtc::PeerConnection>(config);
    rtc::Description::Audio newAudioTrack("audio", rtc::Description::Direction::SendOnly);
    // newAudioTrack.addCodec(0);
    // newAudioTrack.addOpusCodec(111);
    // newAudioTrack.addSSRC(12345678, "audioStream");

    peerConnection->onLocalDescription([this](rtc::Description sdp) {
        if (ongoingSession.has_value()) {
            const auto offerEvent = new RTCOfferSentEvent(sdp, ongoingSession.value());
            meloWebSocketService.sendMessage(offerEvent->createMessage());
            monitorAnswer();
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
                startAudioThread();
                break;
            }
            default: break;
        }
    });
    audioTrack = peerConnection->addTrack(static_cast<rtc::Description::Media>(newAudioTrack));
    setOffer();
}

void MeloWebRTCServerService::resetConnection() {
    if (peerConnection) {
        peerConnection->close();
        peerConnection.reset();
    }

    setupConnection();
}


void MeloWebRTCServerService::startAudioThread() {
    juce::Logger::outputDebugString("Starting audio thread");
    // Crée un thread pour envoyer des données audio
    audioThread = std::thread([this]() {
        while (!stopThread) {
            std::vector<int16_t> pcmData;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                queueCondition.wait(lock, [this]() { return !audioQueue.empty() || stopThread; });
                if (stopThread) break;

                pcmData = std::move(audioQueue.front());
                audioQueue.pop();
            }

            juce::Logger::outputDebugString("Sending audio data");
            if (audioTrack) {
                audioTrack->send(reinterpret_cast<const std::byte*>(pcmData.data()), pcmData.size() * sizeof(int16_t));
            }
        }
    });
}

void MeloWebRTCServerService::stopAudioThread() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopThread = true;
    }
    queueCondition.notify_all();
    if (audioThread.joinable()) {
        audioThread.join();
    }
}

void MeloWebRTCServerService::onWsMessageReceived(const MessageWsReceivedEvent &event) {
    if (event.type == "answer" && event.data.contains("answerSdp")) {
        handleAnswer(event.data["answerSdp"]);
    } else if (event.type == "candidate") {
        std::string candidate = event.data["candidate"];
        std::string sdpMid = event.data["sdpMid"];
        // int sdpMLineIndex = event.data["sdpMLineIndex"];

        rtc::Candidate iceCandidate(rtc::Candidate(candidate, sdpMid));
        peerConnection->addRemoteCandidate(iceCandidate);
    }
}



void MeloWebRTCServerService::setOffer() const {
    peerConnection->setLocalDescription(rtc::Description::Type::Offer);
}

void MeloWebRTCServerService::handleAnswer(const std::string &sdp) {
    peerConnection->setRemoteDescription(rtc::Description(sdp, "answer"));
    answerReceived = true;
}

void MeloWebRTCServerService::onOngoingSessionChanged(const OngoingSessionChangedEvent &event) {
    this->ongoingSession = event.ongoingSession;
    meloWebSocketService.connectToServer();
    this->setupConnection();
};

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

void MeloWebRTCServerService::monitorAnswer() {
    if (!answerTimer.has_value()) {
        answerTimer.emplace(ReconnectTimer([this]() {
            if (answerReceived) {
                juce::Logger::outputDebugString("Answer received. Stopping the timer.");
                answerTimer->stopTimer(); // Arrêter le timer si la réponse est reçue
                return;
            }

            if (resendAttempts >= maxResendAttempts) {
                juce::Logger::outputDebugString("Max resend attempts reached. Stopping resends.");
                answerTimer->stopTimer(); // Arrêter le timer après trop de tentatives
                return;
            }

            // Réémettre l'offre
            juce::Logger::outputDebugString("Resending offer...");
            resetConnection();
            resendAttempts++;
        }));
    }
    // Réinitialiser les variables de suivi
    answerReceived = false;
    resendAttempts = 0;

    // Lancer un timer pour surveiller la réponse
    answerTimer->startTimer(resendIntervalMs);
}