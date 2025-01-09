//
// Created by Padoa on 08/01/2025.
//
#include "MainWindow.h"

MainWindow::MainWindow(const juce::String& name): DocumentWindow(name,juce::Colours::lightgrey, DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    setContentOwned(new juce::Label("label", "Hello, World!"), true);
    setResizable(true, true);
    setSize(600, 400);

    const auto accessToken = JuceLocalStorage::getInstance().loadValue("access_token");
    juce::Logger::outputDebugString("Access Token From Main Page: " + accessToken);
    if (accessToken.isEmpty()) {
        navigateToLoginPage();
    }
    else {
        AuthService::getInstance().fetchUserContext();
        juce::Logger::outputDebugString("Got my user context From Main Page: " + AuthService::getInstance().getUserContext()->user.firstname);
        navigateToMainPage();
    }

    Component::setVisible (true);
    EventManager::getInstance().addListener(this);
}

MainWindow::~MainWindow() {
    EventManager::getInstance().removeListener(this);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
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
    juce::Logger::outputDebugString("Navigating to Login Page");
    currentPage = std::make_unique<LoginPageComponent>();
    addAndMakeVisible(currentPage.get());
    resized(); // Réorganise la disposition
}

void MainWindow::navigateToMainPage()
{
    juce::Logger::outputDebugString("Navigating to Main Page");
    currentPage = std::make_unique<MainPageComponent>();
    addAndMakeVisible(currentPage.get());
    resized(); // Réorganise la disposition
}