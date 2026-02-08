#include <JuceHeader.h>

#include "AcousticView.h"
#include "ui/Style.h"
#include "PluginEditor.h"
#include "State.h"
#include "BeamFormer.h"

#include <iostream>

//==============================================================================

AcousticView::AcousticView(PluginEditor& r) : root(r), state(r.state), fstate(r.fstate)
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
    };

    graph.on_mouse_exit = [this]([[maybe_unused]] const juce::MouseEvent& e){
        dragging = false;
    };

    graph.paint_graph = [this](juce::Graphics &g){

        auto colorMap = [](float x){
            return juce::Colour(0xFF000080).interpolatedWith(juce::Colour(0xFFFFFF00), x);
        };

        int gridN = 63;
        std::vector<vec3> directions(gridN*gridN);

        auto rect = graph.getLocalBounds();
        float width = (float)rect.getWidth();
        float height = (float)rect.getHeight();

        for(int i=0; i<gridN; i++){
            for(int j=0; j<gridN; j++){
                directions[i*gridN+j] = {
                    10,
                    graph.virtual_x(width*(i+0.5f)/gridN),
                    graph.virtual_y(height*(j+0.5f)/gridN)};
            }
        }

        auto energyMap = beamform(fstate, state.micPositions, directions);

        float wstep = width / gridN;
        float hstep = height / gridN;
        for(int i=0; i<gridN; i++){
            for(int j=0; j<gridN; j++){
                g.setColour(colorMap(std::pow(energyMap[i*gridN+j], 10)));
                g.fillRect(juce::Rectangle{i*wstep-0.5f, j*hstep-0.5f, wstep+1.0f, hstep+1.0f});
            }
        }

        float r = 24;
        g.setColour(palette.lwhite);
        g.drawEllipse(graph.canvas_x(state.targetPosition.y)-r/2, graph.canvas_y(state.targetPosition.z)-r/2, r, r, 3);
        r -= 4;
        g.setColour(palette.bg);
        g.drawEllipse(graph.canvas_x(state.targetPosition.y)-r/2, graph.canvas_y(state.targetPosition.z)-r/2, r, r, 3);

        graph.repaint();
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
