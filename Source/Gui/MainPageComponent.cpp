//
// Created by Padoa on 08/01/2025.
//
#include "MainPageComponent.h"

MainPageComponent::MainPageComponent() {
    addAndMakeVisible(title);
    addAndMakeVisible(logoutButton);
    addAndMakeVisible(mainText);

    auto userContext = AuthService::getInstance().getUserContext();
    title.setText(juce::String::fromUTF8(("Bienvenue " + userContext->user.firstname).c_str()), juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setFont(30.0f);

    mainText.setText(juce::String::fromUTF8(("L'audio s'affichera ici")), juce::dontSendNotification);
    mainText.setJustificationType(juce::Justification::centred);

    logoutButton.setButtonText(juce::String::fromUTF8("Se déconnecter"));
    logoutButton.onClick = [this] { onLogoutButtonClick(); };

    EventManager::getInstance().addListener(this);
    meloWebRTCServerService = MeloWebRTCServerService();
    auto res = MeloApiService::getInstance().makeGETRequest(ApiRoute::GetMyOngoingSessions);
    ongoingSessions = PopulatedSession::parseArrayFromJsonString(res);
    if (ongoingSessions.size() > 0) {
        mainText.setText("Vous avez une session en cours :" + ongoingSessions[0].startDate, juce::dontSendNotification);
    }
}

MainPageComponent::~MainPageComponent() {
    logoutButton.onClick = nullptr;
    EventManager::getInstance().removeListener(this);
}

void MainPageComponent::resized() {
    juce::FlexBox pageFlexbox;
    pageFlexbox.flexDirection = juce::FlexBox::Direction::column;
    pageFlexbox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    pageFlexbox.alignContent = juce::FlexBox::AlignContent::center;
    pageFlexbox.items.add(juce::FlexItem(logoutButton).withFlex(0).withHeight(30).withWidth(70).withMargin(juce::FlexItem::Margin(5, 5, 0, 0)).withAlignSelf(juce::FlexItem::AlignSelf::flexEnd));
    pageFlexbox.items.add(juce::FlexItem(title).withFlex(0).withHeight(30).withMargin(juce::FlexItem::Margin(30, 0, 30, 0)));
    pageFlexbox.items.add(juce::FlexItem(mainText).withFlex(1).withMargin(juce::FlexItem::Margin(30, 0, 0, 0)));
    pageFlexbox.performLayout(getLocalBounds());
}

void MainPageComponent::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    for (int channel = 0; channel < event.totalNumInputChannels; ++channel)
    {
        auto* channelData = event.buffer.getWritePointer (channel);
        // meloWebRTCServerService.sendAudioData(channelData, event.buffer.getNumSamples());
        // mainText.setText(juce::String(*channelData), juce::dontSendNotification);
    }
}

void MainPageComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::transparentWhite);
}

void MainPageComponent::onLogoutButtonClick() {
    AuthService::logout();
}