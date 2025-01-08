#pragma once
#include <juce_core/juce_core.h>

namespace Utils {
    static juce::String convertStringPairArrayToPOSTData(const juce::StringPairArray& parameters)
    {
        juce::StringArray parameterStrings;

        for (const auto& key : parameters.getAllKeys())
        {
            auto value = parameters[key];
            parameterStrings.add(juce::URL::addEscapeChars(key, true) + "=" + juce::URL::addEscapeChars(value, true));
        }

        return parameterStrings.joinIntoString("&");
    }
}
