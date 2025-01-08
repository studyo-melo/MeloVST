//
// Created by Padoa on 08/01/2025.
//
//
// Created by Padoa on 08/01/2025.
//
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class MainPageComponent: public juce::Component
{
public:
    MainPageComponent() {
        addAndMakeVisible(label);
        label.setText("Bienvenue sur la Main Page", juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
    }
    ~MainPageComponent() override = default;

    void resized() override {
        label.setBounds(getLocalBounds());
    }

private:
    juce::Label label;
};