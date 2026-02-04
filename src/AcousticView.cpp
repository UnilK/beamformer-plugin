#include <JuceHeader.h>

#include "AcousticView.h"
#include "ui/Style.h"
#include "PluginEditor.h"
#include "State.h"

#include <iostream>

//==============================================================================

AcousticView::AcousticView(PluginEditor& r) : root(r), state(r.state)
{
    addAndMakeVisible(graph);

    graph.set_view(-1, 1, -1, 1);

    graph.on_mouse_down = [this]([[maybe_unused]] const juce::MouseEvent& e){
        dragging = true;
    };

    graph.on_mouse_up = [this]([[maybe_unused]] const juce::MouseEvent& e){
        dragging = false;
    };

    graph.on_mouse_move = [this]([[maybe_unused]] const juce::MouseEvent& e){
        if(!dragging) return;
        state.targetPosition = {1, graph.virtual_x(graph.mouseX), graph.virtual_y(graph.mouseY)};
        graph.repaint();
    };

    graph.on_mouse_exit = [this]([[maybe_unused]] const juce::MouseEvent& e){
        dragging = false;
    };

    graph.paint_graph = [this](juce::Graphics &g){
        const float r = 20;
        g.fillAll (palette.bg);
        g.setColour(palette.lwhite);
        g.drawEllipse(graph.canvas_x(state.targetPosition.y), graph.canvas_y(state.targetPosition.z), r, r, 3);
    };
}

AcousticView::~AcousticView()
{
}

void AcousticView::paint (juce::Graphics& g)
{
    g.fillAll (palette.bg);
}

void AcousticView::resized()
{
    juce::FlexBox fb;
    fb.items.add(juce::FlexItem(graph).withFlex(1,1,0));

    fb.performLayout(getLocalBounds());
}
