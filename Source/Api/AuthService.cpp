#include "AuthService.h"

#include "../Events/EventManager.h"

AuthService &AuthService::getInstance() {
    static AuthService instance;
    return instance;
}

juce::String AuthService::login(const juce::String &email, const juce::String &password) {
    nlohmann::json jsonBody = {
        {"email",    email.toStdString()},
        {"password", password.toStdString()}
    };

    try {
        auto res = ApiService::getInstance().makePOSTRequest(ApiRoute::PostLogin, jsonBody);
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
    auto myUserContext = ApiService::getInstance().makeGETRequest(ApiRoute::GetMyUserContext);
    if (myUserContext.isEmpty()) {
        juce::Logger::outputDebugString("Aucun UserContext trouvé");
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
