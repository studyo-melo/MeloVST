#pragma once
#include <juce_core/juce_core.h>
#include "Events.h"

class EventListener
{
public:
    virtual ~EventListener() = default;
    virtual void onAudioBlockProcessedEvent(const AudioBlockProcessedEvent& event) {}
    virtual void onLoginEvent(const LoginEvent& event) {}
    virtual void onLogoutEvent(const LogoutEvent& event) {}
    virtual void onOngoingSessionChanged(const OngoingSessionChangedEvent& event) {}
    virtual void onWsMessageReceived(const MessageWsReceivedEvent& event) {}
};
