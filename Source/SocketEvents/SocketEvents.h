#pragma once

#include <string>
#include "../Utils/StringUtils.h"

class SellerConnectedEvent {
public:
    SellerConnectedEvent(std::string sellerUserId, std::string sessionId, std::string artistUserId) : sellerUserId(sellerUserId), sessionId(sessionId), artistUserId(artistUserId) {}
    std::string type = "seller-connected";
    std::string sellerUserId;
    std::string sessionId;
    std::string artistUserId;
    std::string toJsonString() {
        return R"({ "sellerUserId": ")" + sellerUserId + R"(", "sessionId": ")" + sessionId + R"(", "artistUserId": ")" + artistUserId + R"(" })";
    }
    std::string createMessage() {
        return  StringUtils::createWsMessage(type, this->toJsonString());
    }
};