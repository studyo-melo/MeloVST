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
    void handleAnswer(const std::string& sdp);
    void setupConnection();
    void disconnect() const;
    bool isConnected() const;
    bool isConnecting() const;
    void setOffer();
    void onOngoingSessionChanged(const OngoingSessionChangedEvent& event) override;
    void onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) override;
    void sendAudioData();
    void onWsMessageReceived(const MessageWsReceivedEvent &event) override;
    void notifyRTCStateChanged() const;
    void monitorAnswer();
    juce::String getSignalingStateLabel() const;
    juce::String getIceCandidateStateLabel() const;

private:
    std::vector<rtc::Candidate> pendingCandidates;
    std::shared_ptr<rtc::PeerConnection> peerConnection;
    std::shared_ptr<rtc::Track> audioTrack;
    std::shared_ptr<rtc::DataChannel> dataChannel;
    MeloWebSocketService meloWebSocketService;
    std::optional<PopulatedSession> ongoingSession;
    std::thread audioThread;
    std::queue<std::vector<int16_t>> audioQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    bool stopThread;

    void pushAudioBuffer(const float* data, int numSamples);
    void stopAudioThread();
    void startAudioThread();
    void sendCandidateToRemote(const rtc::Candidate& candidate);
    void sendOfferToRemote(const rtc::Description &sdp);

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
    void attemptReconnect();
    void resetConnection();
};
