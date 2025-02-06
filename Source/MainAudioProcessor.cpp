#include "MainAudioProcessor.h"
#include "MainApplication.h"
#include "Utils/AudioSettings.h"

//==============================================================================
MainAudioProcessor::MainAudioProcessor()
     : juce::AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
    resampler = new ResamplerWrapper(44100, 48000, 2);
    AudioSettings::getInstance().setBitDepth(16);
    AudioSettings::getInstance().setOpusSampleRate(48000);
    AudioSettings::getInstance().setSampleRate(44100);
    AudioSettings::getInstance().setBlockSize(getBlockSize());
    AudioSettings::getInstance().setNumChannels(getMainBusNumOutputChannels());

    juce::Logger::outputDebugString("Bit depth: " + std::to_string(AudioSettings::getInstance().getBitDepth()));
    juce::Logger::outputDebugString("Opus Sample rate: " + std::to_string(AudioSettings::getInstance().getOpusSampleRate()));
    juce::Logger::outputDebugString("Sample rate: " + std::to_string(AudioSettings::getInstance().getSampleRate()));
    juce::Logger::outputDebugString("Block Size: " + std::to_string(AudioSettings::getInstance().getBlockSize()));
    juce::Logger::outputDebugString("Num Channels: " + std::to_string(AudioSettings::getInstance().getNumChannels()));

}

MainAudioProcessor::~MainAudioProcessor()
= default;

const juce::String MainAudioProcessor::getProgramName (int index)
{
    if (index == 0) return "MeloVST Send";
    if (index == 1) return "MeloVST Receive";
    return {};
}


//==============================================================================
const juce::String MainAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MainAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MainAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MainAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MainAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MainAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MainAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MainAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

void MainAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void MainAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::Logger::outputDebugString("[prepareToPlay] Sample rate: " + std::to_string(sampleRate));
    juce::Logger::outputDebugString("[prepareToPlay] samplesPerBlock: " + std::to_string(samplesPerBlock));
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void MainAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool MainAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void MainAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    std::vector<float> pcmData;

    pcmData.reserve(static_cast<size_t>(numChannels * numSamples));
    for (int channel = 0; channel < numChannels; ++channel)
    {
        const float* channelData = buffer.getReadPointer(channel);
        if (channelData == nullptr) continue;
        for (int sample = 0; sample < numSamples; ++sample) pcmData.push_back(channelData[sample]);
    }
    
    if (pcmData.size() <= 0) {
        return;
    }

    // Notifie l'EventManager qu'un bloc audio a été traité
    EventManager::getInstance().notifyAudioBlockProcessed(AudioBlockProcessedEvent{
        std::move(pcmData), // Transfert des données pour éviter une copie
        numChannels,
        numSamples,
        getSampleRate()
    });
}


//==============================================================================
bool MainAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MainAudioProcessor::createEditor()
{
    return new MainApplication(*this);
}

//==============================================================================
void MainAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void MainAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MainAudioProcessor();
}
