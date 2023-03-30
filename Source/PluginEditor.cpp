/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <math.h>

//==============================================================================
SpectroAudioProcessorEditor::SpectroAudioProcessorEditor (SpectroAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), spectrogramImage (juce::Image::RGB, 512, 512, true)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    startTimerHz(1000);
    
    setSize (512, 564);
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
    auto bounds = getLocalBounds();
    auto rightChanTextBounds = bounds.removeFromTop(bounds.getHeight() * .05);
    auto leftChanTextBounds = bounds.removeFromBottom(bounds.getHeight() * .05);
     
    g.drawImage (spectrogramImage, bounds.toFloat());
    g.setFont(12.f);
    g.setColour(juce::Colours::white);
    juce::String rightChanText = "Right Channel";
    g.drawText(rightChanText, rightChanTextBounds, juce::Justification::verticallyCentred);
    juce::String leftChanText = "Left Channel";
    g.drawText(leftChanText, leftChanTextBounds, juce::Justification::verticallyCentred);
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

    auto mindB = -100.0f;
    auto maxdB =    0.0f;

    for (int i = 0; i < imageHeight / 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            auto skewedProportionY = 1.0f - std::exp (std::log (1.0f - (float) i / ((float) imageHeight / 2.f)) * 0.2f);
            auto fftDataIndex = juce::jlimit (0, audioProcessor.fftSize / 2, (int) (skewedProportionY * (float) audioProcessor.fftSize * 0.5f));
            auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (audioProcessor.fftData[j][fftDataIndex])
                                                               - juce::Decibels::gainToDecibels ((float) audioProcessor.fftSize)),
                                     mindB, maxdB, 0.0f, 1.0f);
            auto stereoShift = (1 - j) * (imageHeight / 2);
            spectrogramImage.setPixelAt (rightHandEdge, imageHeight - i - stereoShift, juce::Colour::fromHSV (.5f, 1.0f - level, level, 1.0f));
        }
    }

}
