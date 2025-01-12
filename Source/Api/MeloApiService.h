#pragma once

#include "ApiRoutes.h"
#include "../Utils/StringUtils.h"
#include "../Utils/JuceLocalStorage.h"

class MeloApiService
{
public:
    // Méthode pour obtenir les données via HTTPS
    juce::String makeGETRequest(ApiRoute route);
    juce::String makePOSTRequest(ApiRoute route, const nlohmann::json& body);
    static MeloApiService& getInstance();
    MeloApiService(const MeloApiService&) = delete;

private:
    template <typename RequestConfig>
    juce::String makeHttpRequest(ApiRoute route, RequestConfig configureRequest);

    static juce::URL::InputStreamOptions buildOptions();
    juce::String buildApiUrl(ApiRoute route);
    MeloApiService() {}
    MeloApiService& operator=(const MeloApiService&) = delete;
};