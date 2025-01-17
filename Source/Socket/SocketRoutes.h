#pragma once
#include <juce_core/juce_core.h>  // Pour manipuler les chaînes de caractères
#include "../Utils/Constants.h"
#include "../Utils/StringUtils.h"

enum class WsRoute
{
    GetOngoingSession,
    GetOngoingSessionRTC,
};


inline juce::String getWsRouteString(const WsRoute route)
{
    switch (route)
    {
        case WsRoute::GetOngoingSession:            return "/ongoing-session";
        case WsRoute::GetOngoingSessionRTC:            return "/ongoing-session-rtc";
        default:                                    return "";
    }
}