#pragma once
#include <exception>
#include <iostream>
#include <juce_core/juce_core.h>
#include "Api/MeloApiService.h"
#include "Api/AuthService.h"

class CrashHandler {
public:
    static void reportCrash(const std::string& message) {
        nlohmann::json jsonBody = {
            {"message", message},
            {"platformType", "juce_send"},
            {"stackTrace", message},
            {"screenName", "MainApp"},
            { "userId", AuthService::getInstance().getUserContext().value().user._id }
        };
        auto res = MeloApiService::getInstance().makePOSTRequest(ApiRoute::PostLogin, jsonBody);
    }

    static void customTerminateHandler() {
        try {
            std::rethrow_exception(std::current_exception());
        } catch (const std::exception& e) {
            reportCrash(e.what());
        } catch (...) {
            reportCrash("Unknown exception occurred");
        }

        // Appeler le gestionnaire par défaut après votre rapport
        std::abort();
    }
};