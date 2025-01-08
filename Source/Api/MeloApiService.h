#pragma once

#include "ApiRoutes.h"
#include "../Utils/StringUtils.h"

class MeloApiService
{
public:
    // Méthode pour obtenir les données via HTTPS
    juce::String makeGETRequest(ApiRoute route);
    juce::String makePOSTRequest(ApiRoute route, const juce::StringPairArray& body);
    static MeloApiService& getInstance();

private:
    template <typename RequestConfig>
    juce::String makeHttpRequest(ApiRoute route, RequestConfig configureRequest);
    static juce::String buildApiUrl(ApiRoute route);
    MeloApiService() {};
    MeloApiService(const MeloApiService&) = delete;
    MeloApiService& operator=(const MeloApiService&) = delete;
};