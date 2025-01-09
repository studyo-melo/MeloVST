#pragma once
#include <juce_core/juce_core.h>
#include "EventListener.h"

class EventManager
{
public:
    static EventManager& getInstance()
    {
        static EventManager instance; // Instance unique
        return instance;
    }
    void addListener(EventListener* listener)
    {
        listeners.add(listener);
    }

    void removeListener(EventListener* listener)
    {
        listeners.removeFirstMatchingValue(listener);
    }

    void notifyAudioBlockProcessed(const AudioBlockProcessedEvent event)
    {
        for (auto* listener : listeners)
        {
            listener->onAudioBlockProcessedEvent(event);
        }
    }

    void notifyLogin(const juce::String& accessToken)
    {
        const LoginEvent event{ accessToken };
        for (auto* listener : listeners)
        {
            listener->onLoginEvent(event);
        }
    }

    void notifyLogout()
    {
        for (auto* listener : listeners) {
            constexpr LogoutEvent event;
            listener->onLogoutEvent(event);
        }
    }

private:
    juce::Array<EventListener*> listeners;
    EventManager() = default; // Constructeur privé
    ~EventManager() = default; // Destructeur privé
    EventManager(const EventManager&) = delete; // Supprime la copie
    EventManager& operator=(const EventManager&) = delete; // Supprime l'assignation

};
