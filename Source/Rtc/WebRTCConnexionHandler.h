#pragma once

#include <iostream>
#include <rtc/rtc.hpp>
#include "../Utils/VectorUtils.h"
#include <juce_core/juce_core.h>

#include "../Api/WebSocketService.h"
#include "../Common/EventListener.h"
#include "../Models/Session.h"
#include "../Api/SocketEvents.h"
#include "../Common/EventManager.h"
#include "../ThirdParty/json.hpp"
#include "../Common/ReconnectTimer.h"

class WebRTCConnexionHandler : EventListener {
public:
    WebRTCConnexionHandler(WsRoute wsRoute, rtc::Description::Direction trackDirection);
    ~WebRTCConnexionHandler();

    virtual void setupConnection();
    void disconnect() const;

    virtual void resetConnection();

    virtual bool isConnected() const;
    virtual bool isConnecting() const;

    virtual juce::String getSignalingStateLabel() const;
    virtual juce::String getIceCandidateStateLabel() const;

protected:
    std::shared_ptr<rtc::PeerConnection> peerConnection;
    std::shared_ptr<rtc::Track> audioTrack;
    void notifyRTCStateChanged() const;

private:
    std::vector<rtc::Candidate> pendingCandidates;
    std::shared_ptr<rtc::DataChannel> dataChannel;
    WebSocketService meloWebSocketService;
    std::optional<PopulatedSession> ongoingSession;
    rtc::Description::Direction trackDirection;


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
