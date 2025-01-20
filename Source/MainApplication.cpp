#include "MainApplication.h"

MainApplication::MainApplication(MainAudioProcessor &p): AudioProcessorEditor(&p) {
    mainWindow = std::make_unique<MainWindow>("Melo");
    addAndMakeVisible(mainWindow.get());

    std::set_terminate(CrashHandler::customTerminateHandler);
    const auto hostDescription = juce::String(juce::PluginHostType().getHostDescription());
    if (hostDescription.equalsIgnoreCase("Unknown")) {
        juce::AudioDeviceManager deviceManager;
        DebugAudioCallback audioCallback = DebugAudioCallback();

        deviceManager.initialise(0, 2, nullptr, true); // 0 entrées, 2 sorties
        deviceManager.addAudioCallback(&audioCallback);
    }
    setSize(600, 400);
}

MainApplication::~MainApplication() = default;

void MainApplication::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::transparentWhite);
}

void MainApplication::resized() {
    const auto area = getLocalBounds();

    if (mainWindow != nullptr)
        mainWindow->setBounds(area);
}
