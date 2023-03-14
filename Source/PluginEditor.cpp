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
//    setOpaque(true);
    startTimerHz(44100);
    
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
    
    //first drawing method
//     find value range to scale
//    auto maxLevel = juce::FloatVectorOperations::findMinAndMax (audioProcessor.fftData.data(), audioProcessor.fftSize / 2);
//
//    for (auto y = 1; y < imageHeight; ++y)
//    {
//        auto skewedProportionY = 1.0f - std::exp (std::log ((float) y / (float) imageHeight) * 0.2f);
//        auto fftDataIndex = (size_t) juce::jlimit (0, audioProcessor.fftSize / 2, (int) (skewedProportionY * audioProcessor.fftSize / 2));
//        auto level = juce::jmap (audioProcessor.fftData[fftDataIndex], 0.0f, juce::jmax (maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
//
//        spectrogramImage.setPixelAt (rightHandEdge, y, juce::Colour::fromHSV (.5f, (1.0f - level), level, 1.0f));
//    }
    
    // second drawing method
//    std::array<float, 512> newBins;
//    std::fill (newBins.begin(), newBins.end(), 0.0f);
//
//    for (auto i = 1; i < (audioProcessor.fftSize / 2) + 1; ++i)
//    {
//        float skewed = log2(i);
//        int bin = (int) (juce::jmap(skewed, 0.f, log2(audioProcessor.fftSize / 2.f), 1.f, 512.f) - 1.f);
//        float level = sqrt(audioProcessor.fftData[i - 1]) / 50;
//        newBins[bin] += level;
//    }
//
//    for (auto i = 0; i < imageHeight; ++i)
//    {
//        float level = newBins[i];
//        level = juce::jlimit (0.f, 1.f, (float) level);
//        spectrogramImage.setPixelAt (rightHandEdge, imageHeight - i + 1, juce::Colour::fromHSV (.5f, 1.0f, level, 1.0f));
//    }
    
    //third drawing method
    auto mindB = -100.0f;
    auto maxdB =    0.0f;

    for (int i = 0; i < imageHeight; ++i)
    {
        auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) imageHeight) * 0.2f);
        auto fftDataIndex = juce::jlimit (0, audioProcessor.fftSize / 2, (int) (skewedProportionX * (float) audioProcessor.fftSize * 0.5f));
        auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (audioProcessor.fftData[fftDataIndex])
                                                           - juce::Decibels::gainToDecibels ((float) audioProcessor.fftSize)),
                                 mindB, maxdB, 0.0f, 1.0f);

        spectrogramImage.setPixelAt (rightHandEdge, imageHeight - i, juce::Colour::fromHSV (.5f, 1.0f, level, 1.0f));
    }

}
