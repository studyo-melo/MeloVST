//
// Created by Padoa on 08/01/2025.
//
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Api/AuthService.h"

class LoginPageComponent: public juce::Component
{
public:
    LoginPageComponent();
    ~LoginPageComponent() override;
    void onLoginButtonClick() const;
    void resized() override;
    void paint(juce::Graphics &g) override;

private:
    juce::Label usernameLabel, passwordLabel, title;
    juce::TextEditor usernameField, passwordField;
    juce::TextButton loginButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoginPageComponent)
};