#pragma once

#include <JuceHeader.h>

struct Palette {
    juce::Colour bg, lbg;
    juce::Colour red, lred;
    juce::Colour green, lgreen;
    juce::Colour yellow, lyellow;
    juce::Colour blue, lblue;
    juce::Colour violet, lviolet;
    juce::Colour cyan, lcyan;
    juce::Colour white, lwhite;
    juce::Colour gray, lgray;
};

extern Palette palette;
extern float fontSize;

class Style : public juce::LookAndFeel_V4 {

public:

    Style();

    void drawResizableWindowBorder(
        juce::Graphics &g,
        int w, int h,
        const juce::BorderSize<int> &border,
        juce::ResizableWindow &window) override;
    
    juce::Button* createDocumentWindowButton(int buttonType) override;

    void drawDocumentWindowTitleBar (
        juce::DocumentWindow& window, juce::Graphics& g,
        int w, int h, int titleSpaceX, int titleSpaceW,
        const juce::Image* icon, bool drawTitleTextOnLeft) override;

    juce::Typeface::Ptr getTypefaceForFont(const juce::Font &font) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override;

    void drawButtonBackground(
        juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font getLabelFont(juce::Label& label) override;

    void drawCornerResizer(juce::Graphics& g, int w, int h, bool, bool) override;
};



class SliderStyle : public Style {

public:

    void drawLabel (juce::Graphics& g, juce::Label& label) override;

    int getSliderThumbRadius (juce::Slider& slider) override;

    juce::Label* createSliderTextBox(juce::Slider& slider) override;

};



class SliderStyleMiddle : public SliderStyle {

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos,
                                       float minSliderPos,
                                       float maxSliderPos,
                                       const juce::Slider::SliderStyle style, juce::Slider& slider) override;
};



class SelectorButtonStyle : public Style {

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override;

};



class TextEntryStyle : public Style {

public:

    void drawLabel (juce::Graphics& g, juce::Label& label) override;

};
