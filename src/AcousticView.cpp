#include <JuceHeader.h>

#include "AcousticView.h"
#include "ui/Style.h"
#include "PluginEditor.h"
#include "State.h"
#include "BeamFormer.h"

#include <iostream>
#include <list>

//==============================================================================

AcousticView::AcousticView(PluginEditor& r) : root(r), state(r.state), fstate(r.fstate)
{
    std::tuple<juce::ToggleButton*, juce::String, bool&> buttons[] = {
        {&noiseButton, "white noise", state.targetNoise},
        {&sineButton, "pure sine", state.targetSine},
        {&frequencyTrackingButton, "Disable frequency tracking", state.disableFrequencyTracking},
        {&phaseAveragingButton, "Disable phase averaging", state.disablePhaseAveraging},
    };

    for(auto &[a, b, c] : buttons){
        a->onClick = [&c](){ c ^= 1; };
        a->setClickingTogglesState(true);
        a->setToggleState(c, juce::NotificationType::dontSendNotification);
        a->setButtonText(b);
        addAndMakeVisible(a);
    }

    std::tuple<juce::Label*, const char*> labels[] = {
        {&targetLabel, "Target sound control"},
        {&frequencyLabel, "Sine frequency"},
        {&volumeLabel, "Volume"},
        {&algoLabel, "Algorithm controls"},
        {&algoSubLabel, "Validate fancy maths"},
        {&SNRLabel, "1/SNR"},
    };

    for(auto [i, a] : labels){
        addAndMakeVisible(*i);
        i->setText(a, juce::dontSendNotification);
        i->setFont(juce::FontOptions{fontSize});
        i->setJustificationType(juce::Justification::left);
    }

    targetLabel.setFont(juce::FontOptions{24.0f});
    targetLabel.setJustificationType(juce::Justification::centred);
    
    algoLabel.setFont(juce::FontOptions{240.0f});
    algoLabel.setJustificationType(juce::Justification::centred);
    algoSubLabel.setFont(juce::FontOptions{6.0f});

    frequencySlider.setRange(200, 20000, 1);
    frequencySlider.setSkewFactor(0.5f);
    frequencySlider.setTextValueSuffix(" Hz");

    volumeSlider.setRange(-100, 0, 0.1);
    volumeSlider.setSkewFactor(4.0f);
    volumeSlider.setTextValueSuffix(" dB");

    SNRSlider.setRange(0, 10, 0.01);
    SNRSlider.setSkewFactor(0.8f);
    SNRSlider.setTextValueSuffix("");

    std::tuple<juce::Slider*, float&> sliders[] = {
        {&frequencySlider, state.targetFrequency},
        {&volumeSlider, state.outVolumedB},
        {&SNRSlider, state.nsr},
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
        g.drawEllipse(graph.canvas_x(state.outTargetPosition.y)-r/2, graph.canvas_y(state.outTargetPosition.z)-r/2, r, r, 3);
        r -= 4;
        g.setColour(palette.bg);
        g.drawEllipse(graph.canvas_x(state.outTargetPosition.y)-r/2, graph.canvas_y(state.outTargetPosition.z)-r/2, r, r, 3);

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
    std::list<juce::FlexBox> fbvec;
    auto fbs = [&](bool makeNew = false) -> juce::FlexBox& {
        if(makeNew) fbvec.emplace_back();
        return fbvec.back();
    };

    juce::FlexBox fb, fbl;
    fb.items.add(juce::FlexItem(fbl).withFlex(1,1,0).withMargin(8));
    fb.items.add(juce::FlexItem(graph).withFlex(2,2,0));

    fbl.flexDirection = juce::FlexBox::Direction::column;

    {
        fbl.items.add(juce::FlexItem(fbs(1)).withFlex(0,0,30));
        fbs().items.add(juce::FlexItem(volumeLabel).withFlex(0,0,140));
        fbs().items.add(juce::FlexItem(volumeSlider).withFlex(1,1,0));
    }

    fbl.items.add(juce::FlexItem(targetLabel).withFlex(0,0,50));
    fbl.items.add(juce::FlexItem(noiseButton).withFlex(0,0,30));
    fbl.items.add(juce::FlexItem(sineButton).withFlex(0,0,30));

    {
        fbl.items.add(juce::FlexItem(fbs(1)).withFlex(0,0,30));
        fbs().items.add(juce::FlexItem(frequencyLabel).withFlex(0,0,140));
        fbs().items.add(juce::FlexItem(frequencySlider).withFlex(1,1,0));
    }

    {
        fbl.items.add(juce::FlexItem(fbs(1)).withFlex(0,0,30));
        fbs().items.add(juce::FlexItem(SNRLabel).withFlex(0,0,140));
        fbs().items.add(juce::FlexItem(SNRSlider).withFlex(1,1,0));
    }

    fbl.items.add(juce::FlexItem(algoLabel).withFlex(0,0,50));
    fbl.items.add(juce::FlexItem(algoSubLabel).withFlex(0,0,30));
    fbl.items.add(juce::FlexItem(phaseAveragingButton).withFlex(0,0,30));
    fbl.items.add(juce::FlexItem(frequencyTrackingButton).withFlex(0,0,30));

    fb.performLayout(getLocalBounds());

    auto rect = graph.getLocalBounds();
    float aspectRatio = (float)rect.getHeight() / rect.getWidth();
    float len = 10.0f / std::sqrt(2.0f); 
    graph.set_view(-len, len, -len * aspectRatio, len * aspectRatio);
}
