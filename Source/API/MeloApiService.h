#pragma once

#include "ApiRoutes.h"
#include "../Utils.h"

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
    static void handleResponse(const juce::String& response);
    static juce::String buildApiUrl(ApiRoute route);


private:
    MeloApiService();

    MeloApiService(const MeloApiService&) = delete;
    MeloApiService& operator=(const MeloApiService&) = delete;
};