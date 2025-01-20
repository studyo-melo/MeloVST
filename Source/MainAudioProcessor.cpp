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
    juce::Logger::outputDebugString("Prepared to play with sample rate: " + std::to_string(sampleRate) + "Hz, " + std::to_string(samplesPerBlock) + " samples per block");
    const std::string wavFilename = FileUtils::generateTimestampedFilename("output", "wav");
    wavFile = initializeWavFile(wavFilename, sampleRate, getNumOutputChannels(), samplesPerBlock);

    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void MainAudioProcessor::releaseResources()
{
    finalizeWavFile(std::move(wavFile));
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

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // juce::Logger::outputDebugString("Processing audio block: " + std::to_string(numSamples) + " samples, " + std::to_string(numChannels) + " channels");
    // Copie des données PCM
    std::vector<float> pcmData;
    for (int channel = 0; channel < numChannels; ++channel) {
        const float* channelData = buffer.getReadPointer(channel);
        pcmData.insert(pcmData.end(), channelData, channelData + numSamples);
    }
    for (float sample : pcmData) {
        if (sample < -1.0f || sample > 1.0f) {
            std::cerr << "Sample out of range: " << sample << std::endl;
        }
        int16_t intSample = static_cast<int16_t>(std::clamp(sample * 32767.0f, -32768.0f, 32767.0f));
        wavFile.write(reinterpret_cast<const char*>(&intSample), sizeof(int16_t));
    }
    // EventManager::getInstance().notifyAudioBlockProcessed(AudioBlockProcessedEvent{pcmData, numChannels, numSamples, getSampleRate()});
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
