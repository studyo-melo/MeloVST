//
// Created by Padoa on 08/01/2025.
//
#include "MainWindow.h"

MainWindow::MainWindow(const juce::String& name): DocumentWindow(name,juce::Colours::lightgrey, DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    setResizable(true, true);
    setSize(600, 400);

    const auto accessToken = JuceLocalStorage::getInstance().loadValue("access_token");
    juce::Logger::outputDebugString("Access Token From Main Page: " + accessToken);
    if (accessToken.isEmpty()) {
        navigateToLoginPage();
    }
    else {
        navigateToMainPage();
    }

    Component::setVisible (true);
}

MainWindow::~MainWindow() = default;

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void MainWindow::paint(juce::Graphics& g)
{
    g.fillAll (juce::Colours::red);
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hella World!", juce::Component::getLocalBounds(), juce::Justification::centred, 1);
}

void MainWindow::resized()
{
    const auto area = getLocalBounds();

    if (currentPage != nullptr)
        currentPage->setBounds(area);
}

void MainWindow::navigateToLoginPage()
{
    currentPage = std::make_unique<LoginPageComponent>([this]() { navigateToMainPage(); });
    addAndMakeVisible(currentPage.get());
    resized(); // Réorganise la disposition
}

void MainWindow::navigateToMainPage()
{
    currentPage = std::make_unique<MainPageComponent>([this]() { navigateToLoginPage(); });
    addAndMakeVisible(currentPage.get());
    resized(); // Réorganise la disposition
}