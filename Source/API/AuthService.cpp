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
    auto accessToken = Utils::parseJsonStringToKeyPair(res).getValue("access_token", "");
    return accessToken;
}

void AuthService::logout(){
}