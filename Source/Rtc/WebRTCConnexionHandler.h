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
#include "../Utils/RTCUtils.h"
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

    YangIceCandidateState getIceState() const { return iceState; }
    YangRequestType getSignalingState() const { return signalingState; }
    YangRtcConnectionState getConnectionState() const { return connectionState; }
protected:
    YangPeerConnection2* yangPeerConnection;
    void notifyRTCStateChanged() const;

private:
    WebSocketService meloWebSocketService;
    std::optional<PopulatedSession> ongoingSession;
    YangIceCandidateState iceState;
    YangRequestType signalingState;
    YangRtcConnectionState connectionState;


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

    void onOngoingSessionChanged(const OngoingSessionChangedEvent& event) override;
    void onWsMessageReceived(const MessageWsReceivedEvent &event) override;
    void onIceStateChange(void* context,int32_t uid,YangIceCandidateType iceCandidateType,YangIceCandidateState _iceState);
    void monitorAnswer();

    // void sendCandidateToRemote(const rtc::Candidate& candidate);
    void sendOfferToRemote(char* sdp);
    void attemptReconnect();
};
