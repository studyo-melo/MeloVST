#include "MainAudioProcessor.h"
#include "MainApplication.h"
#include "Gui/MainWindow.h"

//==============================================================================
MainApplication::MainApplication (MainAudioProcessor& p):
    AudioProcessorEditor(&p),
    mainWindowInstance(std::make_unique<MainWindow>("Main Window")),
    mainWindow(*mainWindowInstance), processorRef(p)
{
    juce::ignoreUnused(processorRef);
}

MainApplication::~MainApplication() = default;

//==============================================================================
void MainApplication::paint (juce::Graphics& g)
{
    mainWindow.paint(g);
}

void MainApplication::resized()
{
    mainWindow.resized();
}
