#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ui/Style.h"
#include "AcousticView.h"

#include <iostream>

//==============================================================================
PluginEditor::PluginEditor (PluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    style = std::make_unique<Style>();
    view = std::make_unique<AcousticView>(*this);
    juce::LookAndFeel::setDefaultLookAndFeel(style.get());
    setLookAndFeel(style.get());

    setSize (1280, 720);
    setResizable(true, true);

    addAndMakeVisible(*view);
}

PluginEditor::~PluginEditor()
{
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (palette.bg);
}

void PluginEditor::resized()
{
    juce::FlexBox fb;
    fb.items.add(juce::FlexItem(*view).withFlex(1,1,0));

    fb.performLayout(getLocalBounds());
}
