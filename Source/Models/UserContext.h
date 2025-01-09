#pragma once

#include <optional>
#include "Artist.h"
#include "User.h"
#include "Seller.h"
#include "../Utils/StringUtils.h"

struct UserContext
{
    std::optional<Artist> artist;
    std::optional<Seller> seller;
    User user;

    UserContext fromJsonString(const juce::String& jsonString) {
        const auto keyPair = juce::JSON::parse(jsonString);

        UserContext userContext;
        userContext.user = User::fromJsonString(juce::JSON::toString(keyPair.getProperty("user", "{}")));
        userContext.seller = Seller::fromJsonString(juce::JSON::toString(keyPair.getProperty("seller", "{}")));
        userContext.artist = Artist::fromJsonString(juce::JSON::toString(keyPair.getProperty("artist", "{}")));
        return userContext;
    }
};