#pragma once
#include <juce_core/juce_core.h>

namespace StringUtils {
    static juce::String convertStringPairArrayToPOSTData(const juce::StringPairArray& parameters)
    {
        juce::StringArray parameterStrings;

        for (const auto& key : parameters.getAllKeys())
        {
            const auto& value = parameters[key];
            parameterStrings.add(juce::URL::addEscapeChars(key, true) + "=" + juce::URL::addEscapeChars(value, true));
        }

        return parameterStrings.joinIntoString("&");
    }

    static juce::StringPairArray parseJsonStringToKeyPair(const juce::String& jsonString)
    {
        juce::StringPairArray keyPairArray;

        // Parse la chaîne JSON en un objet var
        const juce::var parsedJson = juce::JSON::parse(jsonString);

        if (auto* jsonObject = parsedJson.getDynamicObject())
        {
            // Parcours des propriétés avec begin() et end()
            for (const auto & it : jsonObject->getProperties())
            {
                juce::String key = it.name.toString();
                juce::String value = it.value.toString();
                keyPairArray.set(key, value);
            }
        }
        else
        {
            juce::Logger::writeToLog("Invalid JSON or not an object: " + jsonString);
        }

        return keyPairArray;
    }
}
