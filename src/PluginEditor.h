#pragma once

#include "PluginProcessor.h"

class Style;
class AcousticView;

//==============================================================================
class PluginEditor final : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginAudioProcessor&);
    ~PluginEditor() override;

    std::unique_ptr<Style> style;
    std::unique_ptr<AcousticView> view;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginAudioProcessor& processorRef;

    juce::Slider finePitchSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
