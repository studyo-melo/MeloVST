//
// Created by Padoa on 08/01/2025.
//
#include "MainPageComponent.h"

MainPageComponent::MainPageComponent(std::function<void()> onLogout): onLogoutCallback(std::move(onLogout)) {
    auto area = getLocalBounds();
    int fieldHeight = 40;

    // logoutButton.setBounds(area.removeFromTop(fieldHeight));
    logoutButton.setButtonText(juce::String::fromUTF8("Se déconnecter"));
    logoutButton.onClick = [this] { onLogoutButtonClick(); };

    addAndMakeVisible(logoutButton);
    Component::setVisible(true);
}

void MainPageComponent::resized() {
    juce::FlexBox flexbox;
    flexbox.flexDirection = juce::FlexBox::Direction::row; // Disposition horizontale
    flexbox.justifyContent = juce::FlexBox::JustifyContent::spaceBetween; // Espacement entre les éléments

    // Ajouter des FlexItems pour chaque composant
    flexbox.items.add(juce::FlexItem(logoutButton).withFlex(1.0f)); // Proportion 1

    // Appliquer le layout
    flexbox.performLayout(getLocalBounds());
}

void MainPageComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::aliceblue);
}

void MainPageComponent::onLogoutButtonClick() const {
    AuthService::getInstance().logout();
    onLogoutCallback();
}