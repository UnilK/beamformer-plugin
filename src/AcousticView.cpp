#include <JuceHeader.h>

#include "AcousticView.h"
#include "ui/Style.h"
#include "PluginEditor.h"

#include <iostream>

//==============================================================================

AcousticView::AcousticView(PluginEditor& r) : root(r)
{
}

AcousticView::~AcousticView()
{
}

void AcousticView::paint (juce::Graphics& g)
{
    g.fillAll (palette.bg);

    g.setColour (palette.lwhite);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void AcousticView::resized()
{

}
