#include "Style.h"

void Style::drawResizableWindowBorder(
    juce::Graphics &g, [[maybe_unused]]  int w, [[maybe_unused]]  int h,
    [[maybe_unused]] const juce::BorderSize<int> &border,
    [[maybe_unused]] juce::ResizableWindow &window)
{
    g.fillAll(palette.bg);
}

class DocumentWindowButton final : public juce::Button
{
public:

    DocumentWindowButton (const juce::String& name, juce::Colour c, const juce::Path& normal, const juce::Path& toggled)
        : Button (name), colour (c), normalShape (normal), toggledShape (toggled)
    {
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        using namespace juce;
        auto background = Colours::grey;

        if (auto* rw = findParentComponentOfClass<ResizableWindow>())
            if (auto lf = dynamic_cast<LookAndFeel_V4*> (&rw->getLookAndFeel()))
                background = lf->getCurrentColourScheme().getUIColour (LookAndFeel_V4::ColourScheme::widgetBackground);

        g.fillAll (background);

        g.setColour ((! isEnabled() || shouldDrawButtonAsDown) ? colour.withAlpha (0.6f)
                                                     : colour);

        if (shouldDrawButtonAsHighlighted)
        {
            g.fillAll();
            g.setColour (background);
        }

        auto& p = getToggleState() ? toggledShape : normalShape;

        auto reducedRect = Justification (Justification::centred)
                              .appliedToRectangle (Rectangle<int> (getHeight(), getHeight()), getLocalBounds())
                              .toFloat()
                              .reduced ((float) getHeight() * 0.3f);

        g.fillPath (p, p.getTransformToScaleToFit (reducedRect, true));
    }

private:

    juce::Colour colour;
    juce::Path normalShape, toggledShape;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocumentWindowButton)
};

juce::Button* Style::createDocumentWindowButton(int buttonType){
    
    using namespace juce;

    Path shape;
    auto crossThickness = 0.15f;

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment ({ 0.0f, 0.0f, 1.0f, 1.0f }, crossThickness);
        shape.addLineSegment ({ 1.0f, 0.0f, 0.0f, 1.0f }, crossThickness);

        return new DocumentWindowButton("close", palette.lred, shape, shape);
    }

    if (buttonType == DocumentWindow::minimiseButton)
    {
        shape.addLineSegment ({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);

        return new DocumentWindowButton("minimise", palette.lwhite, shape, shape);
    }

    if (buttonType == DocumentWindow::maximiseButton)
    {
        shape.addLineSegment ({ 0.5f, 0.0f, 0.5f, 1.0f }, crossThickness);
        shape.addLineSegment ({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);

        Path fullscreenShape;
        fullscreenShape.startNewSubPath (45.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 45.0f);
        fullscreenShape.addRectangle (45.0f, 45.0f, 100.0f, 100.0f);
        PathStrokeType (30.0f).createStrokedPath (fullscreenShape, fullscreenShape);

        return new DocumentWindowButton("maximise", palette.lwhite, shape, fullscreenShape);
    }

    jassertfalse;
    return nullptr;
}

void Style::drawDocumentWindowTitleBar (
    [[maybe_unused]]  juce::DocumentWindow& window, juce::Graphics& g,
    int w, int h, [[maybe_unused]]  int titleSpaceX, [[maybe_unused]]  int titleSpaceW,
    [[maybe_unused]]  const juce::Image* icon, [[maybe_unused]]  bool drawTitleTextOnLeft)
{
    using namespace juce;

    if (w * h == 0)
        return;

    g.setColour (getCurrentColourScheme().getUIColour (ColourScheme::widgetBackground));
    g.fillAll();
}