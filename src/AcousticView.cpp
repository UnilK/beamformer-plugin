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

    graph.on_mouse_down = [this]([[maybe_unused]] const juce::MouseEvent& e){
        dragging = true;
        graph.on_mouse_move(e);
    };

    graph.on_mouse_up = [this]([[maybe_unused]] const juce::MouseEvent& e){
        dragging = false;
    };

    graph.on_mouse_move = [this]([[maybe_unused]] const juce::MouseEvent& e){
        if(!dragging) return;
        state.targetPosition.y = graph.virtual_x(graph.mouseX);
        state.targetPosition.z = graph.virtual_y(graph.mouseY);
        graph.repaint();
    };

    graph.on_mouse_exit = [this]([[maybe_unused]] const juce::MouseEvent& e){
        dragging = false;
    };

    graph.paint_graph = [this](juce::Graphics &g){
        const float r = 20;
        g.fillAll (palette.bg);
        g.setColour(palette.lwhite);
        g.drawEllipse(graph.canvas_x(state.targetPosition.y)-r/2, graph.canvas_y(state.targetPosition.z)-r/2, r, r, 3);
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

    auto rect = getLocalBounds();
    float aspectRatio = (float)rect.getHeight() / rect.getWidth();
    graph.set_view(-10, 10, -10 * aspectRatio, 10 * aspectRatio);

    fb.performLayout(getLocalBounds());
}
