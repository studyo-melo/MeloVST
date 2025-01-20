#include "MainApplication.h"

MainApplication::MainApplication (MainAudioProcessor& p): AudioProcessorEditor(&p), processorRef(p) {
    std::set_terminate(CrashHandler::customTerminateHandler);
    juce::ignoreUnused(processorRef);

    const auto hostDescription = juce::String(juce::PluginHostType().getHostDescription());
    if (hostDescription.equalsIgnoreCase("Unknown")) {
        juce::AudioDeviceManager deviceManager;
        DebugAudioCallback audioCallback = DebugAudioCallback();

        deviceManager.initialise(0, 2, nullptr, true); // 0 entrées, 2 sorties
        deviceManager.addAudioCallback(&audioCallback);
    }
    setSize(600, 400);
    setResizable(true, true);
    setVisible(true);
    // setUsingNativeTitleBar(true);
    // setContentOwned(new juce::Label("label", "Hello, World!"), true);
    // setSize(600, 400);

    const auto accessToken = JuceLocalStorage::getInstance().loadValue("access_token");
    if (accessToken.isEmpty()) {
        navigateToLoginPage();
    } else {
        AuthService::getInstance().fetchUserContext();
        navigateToMainPage();
    }

    Component::setVisible(true);
    EventManager::getInstance().addListener(this);
}

MainApplication::~MainApplication() {
    EventManager::getInstance().removeListener(this);
};

void MainApplication::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::transparentBlack);
}

void MainApplication::resized()
{
    const auto area = getLocalBounds();

    if (currentPage != nullptr)
        currentPage->setBounds(area);
}


void MainApplication::closeButtonPressed()
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

void MainApplication::onLoginEvent(const LoginEvent& event) {
    navigateToMainPage();
}

void MainApplication::onLogoutEvent(const LogoutEvent& event) {
    navigateToLoginPage();
}

void MainApplication::navigateToLoginPage()
{
    currentPage = std::make_unique<LoginPageComponent>();
    // setContentOwned(currentPage.get(), true);
    resized();
}

void MainApplication::navigateToMainPage()
{
    currentPage = std::make_unique<MainPageComponent>();
    // setContentOwned(currentPage.get(), true);
    // setContentOwned(currentPage.get());
    resized();
}
