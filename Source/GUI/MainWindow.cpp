//
// Created by Padoa on 08/01/2025.
//
#include "MainWindow.h"

MainWindow::MainWindow(const juce::String& name): DocumentWindow(name,juce::Colours::lightgrey, DocumentWindow::allButtons)
{
    setSize (400, 300);
    setUsingNativeTitleBar (true);
    centreWithSize (300, 200);
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

}