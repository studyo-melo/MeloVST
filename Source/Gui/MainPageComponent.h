//
// Created by Padoa on 08/01/2025.
//
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Api/AuthService.h"
#include "../Events/EventListener.h"
#include "../Events/EventManager.h"
#include "../Rtc/WebRTCAudioSenderService.h"
#include "../Socket/WebSocketService.h"
#include "../Api/ApiRoutes.h"
#include "../Models/Session.h"
#include "../Socket/SocketEvents.h"
#include "../Debug/AudioAppPlayer.h"

class MainPageComponent final : public juce::Component, EventListener
{
public:
    explicit MainPageComponent();
    ~MainPageComponent() override;

    static void onLogoutButtonClick();
    void onRTCStateChanged(const RTCStateChangeEvent &event) override;

    void resized() override;
    void paint(juce::Graphics &g) override;

private:
    juce::Label title, mainText, RTCStateText, RTCIceCandidateStateText, RTCSignalingStateText;
    juce::TextButton logoutButton, connectButton, finalizeButton, createButton;
    WebRTCAudioSenderService webRTCAudioService;
    // AudioAppPlayer audioAppPlayer;
    WebSocketService webSocketService;
    juce::Array<PopulatedSession> ongoingSessions;
    PopulatedSession currentOngoingSession;
};