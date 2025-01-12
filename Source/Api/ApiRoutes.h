#pragma once
#include <juce_core/juce_core.h>  // Pour manipuler les chaînes de caractères
#include "../Utils/Constants.h"
#include "../Utils/StringUtils.h"

// Enumération des routes API
enum class ApiRoute
{
    PostLogin,
    GetMyUserContext,
    GetMyOngoingSessions,
    GetHealth
};

// Fonction pour obtenir le nom de la route sous forme de chaîne
inline juce::String getApiRouteString(const ApiRoute route)
{
    switch (route)
    {
        case ApiRoute::PostLogin:             return "/auth/login";
        case ApiRoute::GetMyUserContext:      return "/user-contexts";
        case ApiRoute::GetMyOngoingSessions:      return "/sessions/find-my-ongoing-sessions";
        case ApiRoute::GetHealth:             return "/health";
        default:                              return "";
    }
}

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