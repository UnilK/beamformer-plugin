#pragma once

#include <JuceHeader.h>

#include "ui/Graph.h"

class PluginEditor;
struct State;
struct FilterState;

class AcousticView  : public juce::Component
{
public:

    AcousticView(PluginEditor& r);
    ~AcousticView() override;

    PluginEditor& root;
    State& state;
    FilterState& fstate;

    void paint (juce::Graphics&) override;
    void resized() override;

    bool dragging = false;
    Graph graph;

    juce::ToggleButton noiseButton, sineButton, phaseAveragingButton, frequencyTrackingButton;
    juce::Slider frequencySlider, volumeSlider, SNRSlider;
    juce::Label targetLabel, frequencyLabel, volumeLabel, algoLabel, algoSubLabel, SNRLabel;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcousticView)
};
