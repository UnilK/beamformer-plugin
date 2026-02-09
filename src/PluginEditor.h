#pragma once

#include "PluginProcessor.h"

class Style;
class AcousticView;
struct State;
struct FilterState;
class SliderStyle;

//==============================================================================
class PluginEditor final : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginAudioProcessor&);
    ~PluginEditor() override;

    State& state;
    FilterState& fstate;

    std::unique_ptr<Style> style;
    std::unique_ptr<AcousticView> view;
    std::unique_ptr<SliderStyle> sliderStyle;

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
