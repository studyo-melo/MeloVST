#include "MainApplication.h"

#include "Config.h"

MainApplication::MainApplication(MainAudioProcessor &p): AudioProcessorEditor(&p) {
#ifdef MELO_PLUGIN_NAME
    mainWindow = std::make_unique<MainWindow>(MELO_PLUGIN_NAME);
#else
    mainWindow = std::make_unique<MainWindow>("MeloVST");
#endif

    addAndMakeVisible(mainWindow.get());

    std::set_terminate(CrashHandler::customTerminateHandler);
    if (const auto hostDescription = juce::String(juce::PluginHostType().getHostDescription()); hostDescription.equalsIgnoreCase("Unknown")) {
        juce::AudioDeviceManager deviceManager;
        auto audioCallback = DebugAudioCallback();

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
