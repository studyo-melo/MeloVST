//
// Created by Padoa on 08/01/2025.
//
//
// Created by Padoa on 08/01/2025.
//
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Api/AuthService.h"
#include "../Events/EventListener.h"
#include "../Events/EventManager.h"
#include "../Api/MeloWebRTCServerService.h"
#include "../Models/Session.h"

class MainPageComponent final : public juce::Component, public EventListener
{
public:
    explicit MainPageComponent();
    ~MainPageComponent() override;

    static void onLogoutButtonClick();
    void onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) override;

    void resized() override;
    void paint(juce::Graphics &g) override;

private:
    juce::Label title, mainText;
    juce::TextButton logoutButton;
    MeloWebRTCServerService meloWebRTCServerService;
    juce::Array<PopulatedSession> ongoingSessions;
};