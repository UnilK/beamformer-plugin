#pragma once

#include <JuceHeader.h>

class PluginEditor;

class AcousticView  : public juce::Component
{
public:

    AcousticView(PluginEditor& r);
    ~AcousticView() override;

    PluginEditor& root;

    void paint (juce::Graphics&) override;
    void resized() override;

    juce::Slider pitchMinSlider;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcousticView)
};
