#include "MeloApiService.h"

MeloApiService &MeloApiService::getInstance() {
    static MeloApiService instance;
    return instance;
}

// Méthode GET
juce::String MeloApiService::makeGETRequest(const ApiRoute route)
{
    return makeHttpRequest(route, [](juce::URL& url, juce::URL::InputStreamOptions& options) {
        options.withHttpRequestCmd("GET");
        // Pas besoin de modifier l'URL pour GET
    });
}

// Méthode POST
juce::String MeloApiService::makePOSTRequest(const ApiRoute route, const juce::StringPairArray& body)
{
    return makeHttpRequest(route, [&body](juce::URL& url, juce::URL::InputStreamOptions& options) {
        options.withHttpRequestCmd("POST");
        const auto postData = StringUtils::convertStringPairArrayToPOSTData(body);
        url = url.withPOSTData(postData);
    });
}


// Méthode HTTP générique
template <typename RequestConfig>
juce::String MeloApiService::makeHttpRequest(const ApiRoute route, RequestConfig configureRequest)
{
    juce::String apiUrl = buildApiUrl(route);
    juce::Logger::outputDebugString("Making HTTP request to " + apiUrl);

    try
    {
        juce::URL url(apiUrl);
        const juce::URL::ParameterHandling parameterHandling{};
        juce::URL::InputStreamOptions options{parameterHandling};

        // Configure l'URL et les options via la lambda
        configureRequest(url, options);

        // Effectue la requête synchronisée
        std::unique_ptr<juce::InputStream> stream(url.createInputStream(options));
        if (stream == nullptr)
        {
            throw std::runtime_error("Unable to create input stream");
        }

        // Lis la réponse et la retourne
        return stream->readEntireStreamAsString();
    }
    catch (const std::exception& e)
    {
        return "Exception lors de la requête HTTP : " + juce::String(e.what());
    }
}

// Méthode pour construire l'URL complète
juce::String MeloApiService::buildApiUrl(const ApiRoute route)
{
    return Constants::apiUrl + getApiRouteString(route);
}
