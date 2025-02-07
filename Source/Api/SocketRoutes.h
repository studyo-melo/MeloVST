#pragma once
#include "../Config.h"
#include "../Utils/StringUtils.h"
#include <juce_core/juce_core.h>

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