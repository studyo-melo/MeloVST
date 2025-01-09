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
    fetchUserContext();
    return accessToken;
}

std::optional<UserContext> AuthService::fetchUserContext() {
    auto myUserContext = MeloApiService::getInstance().makeGETRequest(ApiRoute::GetMyUserContext);
    userContext = UserContext().fromJsonString(myUserContext);
    return userContext;
}

void AuthService::logout(){
    JuceLocalStorage::getInstance().removeValue("access_token");
}

std::optional<UserContext> AuthService::getUserContext() const{
    return userContext;
}