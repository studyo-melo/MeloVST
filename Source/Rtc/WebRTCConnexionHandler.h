#pragma once

#include <iostream>
#include <rtc/rtc.hpp>
#include "../Utils/VectorUtils.h"
#include <juce_core/juce_core.h>
#include <yangrtc/YangPeerConnection2.h>

#include "../Socket/WebSocketService.h"
#include "../Events/EventListener.h"
#include "../Models/Session.h"
#include "../Socket/SocketEvents.h"
#include "../Events/EventManager.h"
#include "../Utils/json.hpp"
#include "ReconnectTimer.h"

#include <yangutil/yangavctype.h>

class WebRTCConnexionHandler : EventListener {
public:
    WebRTCConnexionHandler();
    ~WebRTCConnexionHandler();

    virtual void setupConnection();
    void disconnect() const;

    virtual void resetConnection();

    virtual bool isConnected() const;
    virtual bool isConnecting() const;

    virtual juce::String getSignalingStateLabel() const;
    virtual juce::String getIceCandidateStateLabel() const;

protected:
    YangPeerConnection2* yangPeerConnection;
    std::shared_ptr<rtc::PeerConnection> peerConnection;
    std::shared_ptr<rtc::Track> audioTrack;
    void notifyRTCStateChanged() const;

private:
    std::vector<rtc::Candidate> pendingCandidates;
    std::shared_ptr<rtc::DataChannel> dataChannel;
    WebSocketService meloWebSocketService;
    std::optional<PopulatedSession> ongoingSession;


    // Answer monitoring
    bool answerReceived = false;
    const int maxResendAttempts = 100;
    int resendAttempts = 0;
    int resendIntervalMs = 10000;
    std::optional<ReconnectTimer> answerTimer;

    // Ice Reconnection
    int reconnectAttempts = 0;
    const int maxReconnectAttempts = 5;
    const int reconnectDelayMs = 2000;
    ReconnectTimer reconnectTimer;

    void handleAnswer(const std::string& sdp);
    void setOffer();
    void onOngoingSessionChanged(const OngoingSessionChangedEvent& event) override;
    void onWsMessageReceived(const MessageWsReceivedEvent &event) override;
    void monitorAnswer();

    void sendCandidateToRemote(const rtc::Candidate& candidate);
    void sendOfferToRemote(const rtc::Description &sdp);
    void attemptReconnect();
};
