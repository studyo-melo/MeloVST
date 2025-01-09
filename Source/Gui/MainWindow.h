//
// Created by Padoa on 08/01/2025.
//
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LoginPageComponent.h"
#include "MainPageComponent.h"
#include "../Events/EventListener.h"
#include "../Events/EventManager.h"

class MainWindow final : public juce::DocumentWindow, public EventListener
{
public:
    explicit MainWindow (const juce::String& name);
    ~MainWindow() override;

    void closeButtonPressed() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void onLoginEvent(const LoginEvent &event) override;
    void onLogoutEvent(const LogoutEvent &event) override;

private:
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)

    std::unique_ptr<juce::Component> currentPage;
    void navigateToLoginPage();
    void navigateToMainPage();
};