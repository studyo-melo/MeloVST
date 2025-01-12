#include "MainAudioProcessor.h"
#include "MainApplication.h"
#include "Gui/MainWindow.h"
#include "juce_audio_utils/juce_audio_utils.h"
#include "Debug/DebugAudioCallback.h"
//==============================================================================
MainApplication::MainApplication (MainAudioProcessor& p):
    AudioProcessorEditor(&p),
    mainWindowInstance(std::make_unique<MainWindow>("Main Window")),
    mainWindow(*mainWindowInstance), processorRef(p)
{
    juce::ignoreUnused(processorRef);
    if constexpr (Constants::debug == true) {
        juce::AudioDeviceManager deviceManager;
        DebugAudioCallback audioCallback = DebugAudioCallback();

        deviceManager.initialise(0, 2, nullptr, true); // 0 entrées, 2 sorties
        deviceManager.addAudioCallback(&audioCallback);
    }

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
