#include "MainApplication.h"

MainApplication::MainApplication (MainAudioProcessor& p):
    AudioProcessorEditor(&p),
    mainWindowInstance(std::make_unique<MainWindow>("Melo")),
    mainWindow(*mainWindowInstance), processorRef(p)
{
    std::set_terminate(CrashHandler::customTerminateHandler);
    juce::ignoreUnused(processorRef);

     const auto hostDescription = juce::String(juce::PluginHostType().getHostDescription());
     if (hostDescription.equalsIgnoreCase("Unknown")) {
         juce::AudioDeviceManager deviceManager;
         DebugAudioCallback audioCallback = DebugAudioCallback();

         deviceManager.initialise(0, 2, nullptr, true); // 0 entrées, 2 sorties
         deviceManager.addAudioCallback(&audioCallback);
    }
}

MainApplication::~MainApplication() = default;

void MainApplication::paint (juce::Graphics& g)
{
    mainWindow.paint(g);
}

void MainApplication::resized()
{
    mainWindow.resized();
}
