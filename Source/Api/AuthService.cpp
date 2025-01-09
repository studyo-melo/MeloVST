#include "AuthService.h"

AuthService& AuthService::getInstance(){
    static AuthService instance;
    return instance;
};

juce::String AuthService::login(const juce::String& email, const juce::String& password){
    juce::StringPairArray jsonBody;
    jsonBody.set("email", email);
    jsonBody.set("password", password);

    auto res = MeloApiService::getInstance().makePOSTRequest(ApiRoute::PostLogin, jsonBody);
    auto accessToken = StringUtils::parseJsonStringToKeyPair(res).getValue("access_token", "");
    juce::Logger::outputDebugString("Réponse de la requête : " + accessToken);
    JuceLocalStorage::getInstance().saveValue("access_token", accessToken);
    return accessToken;
}

void AuthService::logout(){
    JuceLocalStorage::getInstance().removeValue("access_token");
}