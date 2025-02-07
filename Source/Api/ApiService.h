#pragma once

#include "ApiRoutes.h"
#include "../Utils/StringUtils.h"
#include "../ThirdParty/json.hpp"
#include "../Common/JuceLocalStorage.h"

class ApiService
{
public:
    // Méthode pour obtenir les données via HTTPS
    juce::String makeGETRequest(ApiRoute route);
    juce::String makePOSTRequest(ApiRoute route, const nlohmann::json& body);
    static ApiService& getInstance();
    ApiService(const ApiService&) = delete;

private:
    template <typename RequestConfig>
    juce::String makeHttpRequest(ApiRoute route, RequestConfig configureRequest);

    static juce::URL::InputStreamOptions buildOptions();
    juce::String buildApiUrl(ApiRoute route);
    ApiService() {}
    ApiService& operator=(const ApiService&) = delete;
};