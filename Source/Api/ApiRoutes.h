#pragma once
#include <juce_core/juce_core.h>  // Pour manipuler les chaînes de caractères
#include "../Utils/Constants.h"
#include "../Utils/StringUtils.h"

// Enumération des routes API
enum class ApiRoute
{
    PostLogin,
    GetHealth
};

// Fonction pour obtenir le nom de la route sous forme de chaîne
inline juce::String getApiRouteString(ApiRoute route)
{
    switch (route)
    {
        case ApiRoute::PostLogin:      return "/auth/login";
        case ApiRoute::GetHealth:      return "/health";
        default:                       return "";
    }
}
