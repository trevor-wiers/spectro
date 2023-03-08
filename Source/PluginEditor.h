/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SpectroAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    SpectroAudioProcessorEditor (SpectroAudioProcessor&);
    ~SpectroAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void pushNextSampleIntoFifo(float sample) noexcept;
    void drawNextLineOfSpectrogram();
    
    static constexpr auto fftOrder = 10;
    static constexpr auto fftSize = 1 << fftOrder;
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SpectroAudioProcessor& audioProcessor;
    
    juce::dsp::FFT forwardFFT;
    juce::Image spectrogramImage;
    
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectroAudioProcessorEditor)
};
