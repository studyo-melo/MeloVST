//
// Created by Padoa on 08/01/2025.
//
#include "MainWindow.h"

MainWindow::MainWindow(const juce::String& name): DocumentWindow(name,juce::Colours::lightgrey, DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    setResizable(true, true);
    setSize(600, 400);

    // Ajoute les boutons de navigation
    addAndMakeVisible(&buttonPage1);
    addAndMakeVisible(&buttonPage2);

    buttonPage1.setButtonText(juce::String("Aller à la page de login"));
    buttonPage2.setButtonText("Aller à la page Main");

    buttonPage1.onClick = [this] { navigateToLoginPage(); };
    buttonPage2.onClick = [this] { navigateToMainPage(); };

    // Initialise avec la Page 1
    navigateToLoginPage();
    setVisible (true);
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
    auto area = getLocalBounds();
    auto buttonHeight = 40;

    buttonPage1.setBounds(area.removeFromTop(buttonHeight).reduced(10));
    buttonPage2.setBounds(area.removeFromTop(buttonHeight).reduced(10));

    if (currentPage != nullptr)
        currentPage->setBounds(area);
}

void MainWindow::navigateToLoginPage()
{
    currentPage = std::make_unique<LoginPageComponent>();
    addAndMakeVisible(currentPage.get());
    resized(); // Réorganise la disposition
}

void MainWindow::navigateToMainPage()
{
    currentPage = std::make_unique<MainPageComponent>();
    addAndMakeVisible(currentPage.get());
    resized(); // Réorganise la disposition
}