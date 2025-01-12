#pragma once

#include <iostream>
#include <rtc/rtc.hpp>
#include "../Utils/VectorUtils.h"
#include <juce_core/juce_core.h>

#include "MeloWebSocketService.h"
#include "../Events/EventListener.h"
#include "../Models/Session.h"
#include "../SocketEvents/SocketEvents.h"
#include "../Events/EventManager.h"
#include "../Utils/json.hpp"
#include "ReconnectTimer.h"

class MeloWebRTCServerService final: EventListener {
public:
    MeloWebRTCServerService();
    ~MeloWebRTCServerService() override;
    void handleAnswer(const std::string& sdp) const;
    void setupConnection();
    void setOffer() const;
    void onOngoingSessionChanged(const OngoingSessionChangedEvent& event) override;
    void onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) override;
    void handleAudioData(const AudioBlockProcessedEvent &event) const;
    void onWsMessageReceived(const MessageWsReceivedEvent &event) override;

private:
    std::shared_ptr<rtc::PeerConnection> peerConnection;
    std::shared_ptr<rtc::Track> audioTrack;
    std::shared_ptr<rtc::DataChannel> dataChannel;
    MeloWebSocketService meloWebSocketService;
    std::optional<PopulatedSession> ongoingSession;


    int reconnectAttempts = 0;
    const int maxReconnectAttempts = 5;
    const int reconnectDelayMs = 2000; // Délai entre les tentatives en millisecondes

    ReconnectTimer reconnectTimer; // Utilisation de juce::Timer pour gérer les délais
    void attemptReconnect();
    void resetConnection();
};
