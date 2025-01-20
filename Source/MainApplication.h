#pragma once

#include "MainAudioProcessor.h"
#include "MainAudioProcessor.h"
#include "Debug/DebugAudioCallback.h"
#include "CrashHandler.h"
#include "juce_audio_utils/juce_audio_utils.h"
#include <memory>
#include "Events/EventListener.h"
#include "Events/EventManager.h"
#include "Events/EventManager.h"
#include "Gui/LoginPageComponent.h"
#include "Gui/MainPageComponent.h"

//==============================================================================
class MainApplication final : public juce::AudioProcessorEditor, public EventListener
{
public:
    explicit MainApplication (MainAudioProcessor&);
    ~MainApplication() override;

    void closeButtonPressed();
    void paint (juce::Graphics&) override;
    void resized() override;
    void onLoginEvent(const LoginEvent &event) override;
    void onLogoutEvent(const LogoutEvent &event) override;

private:
    MainAudioProcessor& processorRef;



    std::unique_ptr<juce::Component> currentPage;
    void navigateToLoginPage();
    void navigateToMainPage();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainApplication)
};
