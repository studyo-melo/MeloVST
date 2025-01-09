//
// Created by Padoa on 08/01/2025.
//
#include "MainPageComponent.h"

MainPageComponent::MainPageComponent() {
    addAndMakeVisible(title);
    addAndMakeVisible(logoutButton);

    auto userContext = AuthService::getInstance().getUserContext();
    title.setText(juce::String::fromUTF8(("Bienvenue " + userContext->user.firstname).c_str()), juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setFont(30.0f);

    logoutButton.setButtonText(juce::String::fromUTF8("Se déconnecter"));
    logoutButton.onClick = [this] { onLogoutButtonClick(); };
}

void MainPageComponent::resized() {
    juce::FlexBox pageFlexbox;
    pageFlexbox.flexDirection = juce::FlexBox::Direction::column;
    pageFlexbox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    pageFlexbox.alignContent = juce::FlexBox::AlignContent::center;
    pageFlexbox.items.add(juce::FlexItem(logoutButton).withFlex(0).withHeight(30).withWidth(70).withMargin(juce::FlexItem::Margin(5, 5, 0, 0)).withAlignSelf(juce::FlexItem::AlignSelf::flexEnd));
    pageFlexbox.items.add(juce::FlexItem(title).withFlex(0).withHeight(30).withMargin(juce::FlexItem::Margin(30, 0, 30, 0)));
    pageFlexbox.performLayout(getLocalBounds());
}

void MainPageComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::transparentWhite);
}

void MainPageComponent::onLogoutButtonClick() const {
    AuthService::getInstance().logout();
}