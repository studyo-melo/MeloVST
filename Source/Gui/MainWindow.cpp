//
// Created by Padoa on 08/01/2025.
//
#include "MainWindow.h"
#include "../Debug/AudioAppPlayer.h"

MainWindow::MainWindow(const juce::String& name): Component(name)
{
    AudioBlockPlayer *audioBlockPlayer = new AudioBlockPlayer();
    addAndMakeVisible(audioBlockPlayer);

    const auto accessToken = JuceLocalStorage::getInstance().loadValue("access_token");
    if (accessToken.isEmpty()) {
        navigateToLoginPage();
    }
    else {
        AuthService::getInstance().fetchUserContext();
        navigateToMainPage();
    }

    Component::setVisible (true);
    EventManager::getInstance().addListener(this);
}

MainWindow::~MainWindow() {
    EventManager::getInstance().removeListener(this);
}

void MainWindow::paint(juce::Graphics& g)
{
    g.fillAll (juce::Colours::transparentBlack);
}

void MainWindow::resized()
{
    const auto area = getLocalBounds();

    if (currentPage != nullptr)
        currentPage->setBounds(area);
}

void MainWindow::onLoginEvent(const LoginEvent& event) {
    navigateToMainPage();
}

void MainWindow::onLogoutEvent(const LogoutEvent& event) {
    navigateToLoginPage();
}

void MainWindow::navigateToLoginPage()
{
    currentPage = std::make_unique<LoginPageComponent>();
    addAndMakeVisible(currentPage.get());
    resized();
}

void MainWindow::navigateToMainPage()
{
    currentPage = std::make_unique<MainPageComponent>();
    addAndMakeVisible(currentPage.get());
    resized();
}
