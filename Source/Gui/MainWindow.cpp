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

void MainWindow::closeButtonPressed()
{
    const auto hostDescription = juce::String(juce::PluginHostType().getHostDescription());
    if (hostDescription.equalsIgnoreCase("Unknown"))
    {
        juce::Logger::outputDebugString("Exiting main program");
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
    else
    {
        setVisible(false);
    }
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
    setContentOwned(currentPage.get(), true);
    // setContentOwned(currentPage.get());
    resized();
}
