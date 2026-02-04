#pragma once

#include <JuceHeader.h>

#include <functional>

struct View {
    float x0 = 0, x1 = 1, y0 = 0, y1 = 1;
};

class Graph : public juce::Component
{
public:

    Graph();
    ~Graph();

    typedef std::function<void(juce::Graphics&)> paintfunc;
    typedef std::function<void(const juce::MouseEvent&)> mousefunc;
    typedef std::function<void(const juce::MouseEvent&, const juce::MouseWheelDetails&)> wheelfunc;
 
    paintfunc paint_graph = [](juce::Graphics&){};
    mousefunc on_mouse_down = [](const juce::MouseEvent&){};
    mousefunc on_mouse_up = [](const juce::MouseEvent&){};
    mousefunc on_mouse_move = [](const juce::MouseEvent&){};
    mousefunc on_mouse_drag = [](const juce::MouseEvent&){};
    mousefunc on_mouse_exit = [](const juce::MouseEvent&){};
    wheelfunc on_mouse_wheel = [](const juce::MouseEvent&, const juce::MouseWheelDetails&){};

    void mouseDown(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;
    void mouseMove(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;
    void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

    void paint(juce::Graphics&) override;
    void resized() override;

    View view;
    void set_view(float x0, float x1, float y0, float y1);

    void draw_line(juce::Graphics &g, float position, bool axisX, float thickness = 1);

    void draw_plot(juce::Graphics &g, int n, const float* x, const float* y, float thickness = 1);

    void draw_dot(juce::Graphics &g, float x, float y, float r);

    void draw_text(juce::Graphics &g, const juce::String& s, float x, float y);

    float canvas_x(float virtualX);

    float canvas_y(float virtualY);

    float virtual_x(float canvasX);

    float virtual_y(float canvasY);

    const float MAX_SCALE = 1e18f;
    const float MIN_SCALE = 1e-18f;
    const float ZOOM_SPEED = 1.1f;

    float width;
    float height;

    float scaleX = 1;
    float scaleY = 1;
    float offsetX = 0;
    float offsetY = 0;

    float mouseX = 0;
    float mouseY = 0;
    bool mousemove = false;
    bool movingView = false;
 
private:

    void update_scale();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Graph)
};