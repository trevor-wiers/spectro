/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectroAudioProcessor::SpectroAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
forwardFFT (fftOrder),
window (fftSize, juce::dsp::WindowingFunction<float>::hann)
{
}

SpectroAudioProcessor::~SpectroAudioProcessor()
{
}

//==============================================================================
const juce::String SpectroAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectroAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpectroAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpectroAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpectroAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectroAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpectroAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpectroAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpectroAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpectroAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpectroAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SpectroAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpectroAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
#endif

void SpectroAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    juce::AudioBuffer<float> monoBuffer;
    monoBuffer.makeCopyOf(buffer, false);
    monoBuffer.addFrom(0, 0, monoBuffer, 1, 0, monoBuffer.getNumSamples());
    monoBuffer.addFrom(1, 0, monoBuffer, 0, 0, monoBuffer.getNumSamples());
    monoBuffer.applyGain(0.5f);
    
    auto* channelData = monoBuffer.getReadPointer(0);
    for (auto i = 0; i < buffer.getNumSamples(); ++i)
    {
        pushNextSampleIntoFifo(channelData[i]);
    }
}

//==============================================================================
bool SpectroAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpectroAudioProcessor::createEditor()
{
    return new SpectroAudioProcessorEditor (*this);
}

//==============================================================================
void SpectroAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SpectroAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void SpectroAudioProcessor::pushNextSampleIntoFifo (float sample) noexcept
{
    // if the fifo contains enough data, set a flag to say
    // that the next line should now be rendered..
    if (fifoIndex == fftSize)
    {
        if (! nextFFTBlockReady)
        {
            juce::zeromem (fftData, sizeof (fftData));
            memcpy (fftData, fifo, sizeof (fifo));
            window.multiplyWithWindowingTable(fftData, fftSize);
            forwardFFT.performFrequencyOnlyForwardTransform (fftData);
            nextFFTBlockReady = true;
        }
        for (auto i = 0; i < fftSize - 512; ++i)
        {
            fifo[i] = fifo[i + 512];
        }
        fifoIndex = fftSize - 512;
//        fifoIndex = 0;
    }
    fifo[(size_t) fifoIndex++] = sample;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectroAudioProcessor();
}
