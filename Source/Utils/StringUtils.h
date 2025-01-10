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
        juce::Logger::outputDebugString("Parsing JSON: " + jsonString);
        // Parse la chaîne JSON en un objet var
        const juce::var parsedJson = juce::JSON::parse(jsonString);

        if (auto* jsonObject = parsedJson.getDynamicObject())
        {
            // Parcours des propriétés avec begin() et end()
            for (const auto & it : jsonObject->getProperties())
            {
                if (it.value.isObject()) {
                    juce::String key = it.name.toString();
                    juce::var value = juce::JSON::toString(it.value);
                    keyPairArray.set(key, value);
                }
                else {
                    juce::String key = it.name.toString();
                    juce::var value = it.value;
                    keyPairArray.set(key, value);
                }
            }
        }
        else
        {
            juce::Logger::writeToLog("Invalid JSON or not an object: " + jsonString);
        }

        return keyPairArray;
    }

    static juce::StringArray parseJsonStringToArray(const juce::String& jsonString)
    {
        juce::StringArray array;
        const juce::var parsedJson = juce::JSON::parse(jsonString);

        if (auto* jsonArray = parsedJson.getArray())
        {
            for (const auto& value : *jsonArray)
            {
                array.add(value.toString());
            }
        }
        else
        {
            juce::Logger::writeToLog("Invalid JSON or not an array: " + jsonString);
        }

        return array;
    }
}
