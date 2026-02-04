#pragma once

#include <JuceHeader.h>

#include "ui/Graph.h"

class PluginEditor;
struct State;

class AcousticView  : public juce::Component
{
public:

    AcousticView(PluginEditor& r);
    ~AcousticView() override;

    PluginEditor& root;
    State& state;

    void paint (juce::Graphics&) override;
    void resized() override;

    bool dragging = false;
    Graph graph;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcousticView)
};
