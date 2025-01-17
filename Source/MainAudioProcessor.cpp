#include "MainAudioProcessor.h"
#include "MainApplication.h"

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

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    constexpr int sampleRate = 48000; // Fréquence d'échantillonnage en Hz
    constexpr int packetDurationMs = 120; // Durée du paquet en ms
    constexpr int numSamplesPerPacket = (sampleRate * packetDurationMs) / 1000; // Nombre d'échantillons par paquet

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        int numSamples = buffer.getNumSamples();
        int samplesProcessed = 0;

        while (samplesProcessed < numSamples)
        {
            int samplesToSend = std::min(numSamplesPerPacket, numSamples - samplesProcessed);

            // Créer un buffer temporaire pour les échantillons à envoyer
            std::vector<float> tempBuffer(channelData + samplesProcessed, channelData + samplesProcessed + samplesToSend);

            // Envoyer les données audio (convertir au format requis si nécessaire)
            EventManager::getInstance().notifyAudioBlockProcessed(AudioBlockProcessedEvent{tempBuffer.data(), samplesToSend});

            samplesProcessed += samplesToSend;
        }
    }

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
