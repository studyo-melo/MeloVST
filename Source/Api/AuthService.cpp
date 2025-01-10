#include "AuthService.h"

#include "../Events/EventManager.h"

AuthService &AuthService::getInstance() {
    static AuthService instance;
    return instance;
}

juce::String AuthService::login(const juce::String &email, const juce::String &password) {
    juce::StringPairArray jsonBody;
    jsonBody.set("email", email);
    jsonBody.set("password", password);

    try {
        auto res = MeloApiService::getInstance().makePOSTRequest(ApiRoute::PostLogin, jsonBody);
        if (res.isEmpty()) {
            return res;
        }
        auto accessToken = StringUtils::parseJsonStringToKeyPair(res).getValue("access_token", "");
        JuceLocalStorage::getInstance().saveValue("access_token", accessToken);
        fetchUserContext();
        EventManager::getInstance().notifyLogin(accessToken);
        return accessToken;
    } catch (std::exception &e) {
        juce::Logger::outputDebugString("Login failed : " + juce::String(e.what()));
        logout();
        return "";
    }
}

std::optional<UserContext> AuthService::fetchUserContext() {
    auto myUserContext = MeloApiService::getInstance().makeGETRequest(ApiRoute::GetMyUserContext);
    juce::Logger::outputDebugString("Réponse de la requête myUserContext : " + myUserContext);
    if (myUserContext.isEmpty()) {
        logout();
        return std::nullopt;
    }
    userContext = UserContext().fromJsonString(myUserContext);
    return userContext;
}

void AuthService::logout() {
    JuceLocalStorage::getInstance().removeValue("access_token");
    EventManager::getInstance().notifyLogout();
}

std::optional<UserContext> AuthService::getUserContext() const {
    return userContext;
}
