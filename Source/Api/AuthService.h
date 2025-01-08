#pragma once

#include <juce_core/juce_core.h>
#include "MeloApiService.h"
#include "../Utils/StringUtils.h"
#include "../Utils/JuceLocalStorage.h"

class AuthService
{
public:
    juce::String login(const juce::String& email, const juce::String& password);
    void logout();
    static AuthService& getInstance();

private:
    AuthService() {};
    AuthService(const AuthService&) = delete;
    AuthService& operator=(const AuthService&) = delete;
};