#pragma once
#include <string>
#include <juce_core/juce_core.h>
#include "UserContext.h"

class Session
{
public:
  std::string _id;
  std::string sellerProductId;
  std::string reservedByArtistId;
  std::string bookingId;
  std::string sellerId;
  std::string startDate;
  std::string endDate;
  std::string stripePaymentId;
  std::string status;
  juce::Array<std::string> files;
  bool isTest;
};

class PopulatedSession : public Session
{
public:
  UserContext reservedByArtist;

  static PopulatedSession fromJsonString(const juce::String& jsonString) {
    auto json = juce::JSON::parse(jsonString);
    auto keyPair = StringUtils::parseJsonStringToKeyPair(jsonString);
    PopulatedSession session;
    session._id = keyPair.getValue("_id", "").toStdString();
    session.sellerProductId = keyPair.getValue("sellerProductId", "").toStdString();
    session.reservedByArtistId = keyPair.getValue("reservedByArtistId", "").toStdString();
    session.bookingId = keyPair.getValue("bookingId", "").toStdString();
    session.sellerId = keyPair.getValue("sellerId", "").toStdString();
    session.startDate = keyPair.getValue("startDate", "").toStdString();
    session.endDate = keyPair.getValue("endDate", "").toStdString();
    session.stripePaymentId = keyPair.getValue("stripePaymentId", "").toStdString();
    session.status = keyPair.getValue("status", "").toStdString();
    session.isTest = keyPair.getValue("isTest", "false").toStdString() == "true";
    session.reservedByArtist = UserContext::fromJsonString(juce::JSON::toString(json.getProperty("reservedByArtist", "{}")));
    return session;
  }

  static juce::Array<PopulatedSession> parseArrayFromJsonString(const juce::String& jsonString) {
    juce::Logger::outputDebugString("Réponse de la requête : " + jsonString);
    auto json = juce::JSON::parse(jsonString);
    auto array = json.getArray();
    juce::Array<PopulatedSession> sessions;
    for (int i = 0; i < array->size(); i++) {
      sessions.add(fromJsonString(juce::JSON::toString(array[i])));
    }
    return sessions;
  }
};