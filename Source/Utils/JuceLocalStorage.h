#include <juce_core/juce_core.h>
#include <memory>
#include "Constants.h"

class JuceLocalStorage
{
public:
    static JuceLocalStorage& getInstance()
    {
        static JuceLocalStorage instance(Constants::appName);
        return instance;
    }

    void saveValue(const juce::String& key, const juce::String& value)
    {
        propertiesFile->setValue(key, value);
        propertiesFile->saveIfNeeded();
        juce::Logger::writeToLog("Value saved for key: " + key);
    }

    juce::String loadValue(const juce::String& key) const
    {
        juce::String value = propertiesFile->getValue(key, {});
        if (value.isNotEmpty())
        {
            juce::Logger::writeToLog("Value loaded for key: " + key + " - " + value);
        }
        else
        {
            juce::Logger::writeToLog("No value found for key: " + key);
        }
        return value;
    }

    void removeValue(const juce::String& key)
    {
        propertiesFile->removeValue(key);
        propertiesFile->saveIfNeeded();
        juce::Logger::writeToLog("Value removed for key: " + key);
    }

private:
    JuceLocalStorage(const juce::String& appName)
    {
        // Configure les options de fichier de propriétés
        juce::PropertiesFile::Options options;
        options.applicationName = appName;
        options.filenameSuffix = ".settings";
        options.osxLibrarySubFolder = "Application Support";

        propertiesFile = std::make_unique<juce::PropertiesFile>(options);
    }

    // Supprime le constructeur de copie et le constructeur d'affectation
    JuceLocalStorage(const JuceLocalStorage&) = delete;
    JuceLocalStorage& operator=(const JuceLocalStorage&) = delete;

    std::unique_ptr<juce::PropertiesFile> propertiesFile;
};
