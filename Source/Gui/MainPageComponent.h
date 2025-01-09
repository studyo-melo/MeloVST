//
// Created by Padoa on 08/01/2025.
//
//
// Created by Padoa on 08/01/2025.
//
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Api/AuthService.h"

class MainPageComponent final : public juce::Component
{
public:
    explicit MainPageComponent(std::function<void()> onLogout);
    ~MainPageComponent() override = default;

    void onLogoutButtonClick() const;

    void resized() override;
    void paint(juce::Graphics &g) override;

private:
    juce::Label title;
    std::function<void()> onLogoutCallback;
    juce::TextButton logoutButton;
};