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
    std::tuple<juce::ToggleButton*, juce::String, bool&> ctrp[2] = {
        {&noiseButton, "white noise", state.targetNoise},
        {&sineButton, "pure sine", state.targetSine},
    };

    for(auto &[a, b, c] : ctrp){
        a->onClick = [&c](){ c ^= 1; };
        a->setClickingTogglesState(true);
        a->setToggleState(c, juce::NotificationType::dontSendNotification);
        a->setButtonText(b);
        addAndMakeVisible(a);
    }

    auto labels = {
        &targetLabel,
        &frequencyLabel
    };

    for(juce::Label *i : labels){
        addAndMakeVisible(*i);
        i->setFont(juce::FontOptions{fontSize});
        i->setJustificationType(juce::Justification::left);
    }

    targetLabel.setFont(juce::FontOptions{24.0f});
    targetLabel.setJustificationType(juce::Justification::centred);

    targetLabel.setText("Target sound control", juce::dontSendNotification);
    frequencyLabel.setText("sine frequency", juce::dontSendNotification);

    frequencySlider.setRange(200, 20000, 1);
    frequencySlider.setSkewFactor(0.5f);
    frequencySlider.setTextValueSuffix(" Hz");

    std::tuple<juce::Slider*, float&> sliders[] = {
        {&frequencySlider, state.targetFrequency},
    };

    for(auto [i, a] : sliders){
        addAndMakeVisible(*i);
        i->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 100, 28);
        i->setLookAndFeel(r.sliderStyle.get());
        i->setValue(a, juce::NotificationType::dontSendNotification);
        i->onValueChange = [&a, i](){
            a = (float)i->getValue();
        };
    }

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
            return palette.bg.interpolatedWith(palette.lyellow, x);
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
                g.setColour(colorMap(std::pow(energyMap[i*gridN+j], 10.0f)));
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
    juce::FlexBox fb, fbl;
    fb.items.add(juce::FlexItem(fbl).withFlex(1,1,0).withMargin(8));
    fb.items.add(juce::FlexItem(graph).withFlex(2,2,0));

    fbl.flexDirection = juce::FlexBox::Direction::column;
    fbl.items.add(juce::FlexItem(targetLabel).withFlex(0,0,50));
    fbl.items.add(juce::FlexItem(noiseButton).withFlex(0,0,30));
    fbl.items.add(juce::FlexItem(sineButton).withFlex(0,0,30));

    juce::FlexBox fbs;
    fbl.items.add(juce::FlexItem(fbs).withFlex(0,0,30));
    fbs.items.add(juce::FlexItem(frequencyLabel).withFlex(0,0,140));
    fbs.items.add(juce::FlexItem(frequencySlider).withFlex(1,1,0));

    fb.performLayout(getLocalBounds());

    auto rect = graph.getLocalBounds();
    float aspectRatio = (float)rect.getHeight() / rect.getWidth();
    graph.set_view(-10, 10, -10 * aspectRatio, 10 * aspectRatio);
}
