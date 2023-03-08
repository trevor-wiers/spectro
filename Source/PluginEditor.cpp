/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectroAudioProcessorEditor::SpectroAudioProcessorEditor (SpectroAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), spectrogramImage (juce::Image::RGB, 512, 512, true)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
//    setOpaque(true);
    startTimerHz(audioProcessor.getSampleRate());
    
    setSize (512, 512);
}

SpectroAudioProcessorEditor::~SpectroAudioProcessorEditor()
{
}

//==============================================================================
void SpectroAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);

    g.setOpacity (1.0f);
    g.drawImage (spectrogramImage, getLocalBounds().toFloat());

}

void SpectroAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void SpectroAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.nextFFTBlockReady)
    {
        drawNextLineOfSpectrogram();
        audioProcessor.nextFFTBlockReady = false;
        repaint();
    }
}

void SpectroAudioProcessorEditor::drawNextLineOfSpectrogram()
{
    auto rightHandEdge = spectrogramImage.getWidth() - 1;
    auto imageHeight = spectrogramImage.getHeight();
    
    //first shuffle image foward
    spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);
    
    // find value range to scale
    auto maxLevel = juce::FloatVectorOperations::findMinAndMax (audioProcessor.fftData.data(), audioProcessor.fftSize / 2);
    
    for (auto y = 1; y < imageHeight; ++y)
    {
        auto skewedProportionY = 1.0f - std::exp (std::log ((float) y / (float) imageHeight) * 0.2f);
        auto fftDataIndex = (size_t) juce::jlimit (0, audioProcessor.fftSize / 2, (int) (skewedProportionY * audioProcessor.fftSize / 2));
        auto level = juce::jmap (audioProcessor.fftData[fftDataIndex], 0.0f, juce::jmax (maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
        
        spectrogramImage.setPixelAt (rightHandEdge, y, juce::Colour::fromHSV (.5f, 1.0f, level, 1.0f));
    }
}
