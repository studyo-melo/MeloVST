//
// Created by Padoa on 08/01/2025.
//
#include "MainPageComponent.h"

MainPageComponent::MainPageComponent(std::function<void()> onLogout): onLogoutCallback(std::move(onLogout)) {
    auto area = getLocalBounds();
    int fieldHeight = 40;

    // logoutButton.setBounds(area.removeFromTop(fieldHeight));
    logoutButton.setButtonText("Se déconnecter");

    addAndMakeVisible(logoutButton);
    Component::setVisible(true);
}

void MainPageComponent::resized() {
}

void MainPageComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::transparentBlack);
}

void MainPageComponent::onLogoutButtonClick() const {
    AuthService::getInstance().logout();
    onLogoutCallback();
}