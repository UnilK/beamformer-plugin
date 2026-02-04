#include "Graph.h"

#include "Style.h"

#include <cmath>

Graph::Graph(){
    resized();
}

Graph::~Graph(){

}

void Graph::mouseDown(const juce::MouseEvent &event){
    on_mouse_down(event);
}

void Graph::mouseUp(const juce::MouseEvent &event){
    on_mouse_up(event);
}

void Graph::mouseMove(const juce::MouseEvent &event){

    auto [x, y] = event.getPosition();
    mouseX = (float)x;
    mouseY = (float)y;

    on_mouse_move(event);
}

void Graph::mouseDrag(const juce::MouseEvent &event){
    
    auto [x, y] = event.getPosition();
    mouseX = (float)x;
    mouseY = (float)y;

    on_mouse_move(event);
}

void Graph::mouseExit(const juce::MouseEvent &event){
    on_mouse_exit(event);
}

void Graph::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel){
    on_mouse_wheel(event, wheel);
}

void Graph::paint(juce::Graphics &g){

    g.fillAll(palette.bg);

    paint_graph(g);

    g.setColour (palette.lwhite);

    float cornerSize = 1.0f;
    auto bounds = getLocalBounds().toFloat().reduced (0.5f, 0.5f);

    juce::Path path;
    path.addRoundedRectangle (bounds.getX(), bounds.getY(),
                                bounds.getWidth(), bounds.getHeight(),
                                cornerSize, cornerSize,
                                true, true, true, true);

    g.setColour (palette.lwhite);
    g.strokePath (path, juce::PathStrokeType (1.0f));
}

void Graph::set_view(float x0, float x1, float y0, float y1){
    view = {x0, x1, y0, y1};
    update_scale();
}

void Graph::resized(){
    
    width = (float)getWidth();
    height = (float)getHeight();

    update_scale();
}

void Graph::draw_line(juce::Graphics &g, float position, bool axisX, float thickness)
{
    if(axisX){
        float x = std::floor(canvas_x(position)) + 0.5f;
        g.drawLine(x, 0, x, height, thickness);
    } else {
        float y = std::floor(canvas_y(position)) + 0.5f;
        g.drawLine(0, y, width, y, thickness);
    }
}

void Graph::draw_plot(juce::Graphics &g, int n, const float* x, const float* y, float thickness)
{
    juce::Path path;
    for(int i=1; i<n; i++){
        path.addLineSegment(
            juce::Line<float>(
                canvas_x(x[i-1]),
                canvas_y(y[i-1]),
                canvas_x(x[i]),
                canvas_y(y[i])),
            thickness);
    }

    g.fillPath(path);
}

void Graph::draw_dot(juce::Graphics &g, float x, float y, float r)
{
    g.fillEllipse(canvas_x(x)-r, canvas_y(y)-r, 2*r, 2*r);
}

void Graph::draw_text(juce::Graphics &g, const juce::String& s, float x, float y)
{
    g.drawSingleLineText(s, (int)std::round(canvas_x(x)), (int)std::round(canvas_y(y)));
}

float Graph::canvas_x(float virtualX){
    return (virtualX+offsetX)*scaleX;
}

float Graph::canvas_y(float virtualY){
    return height - (virtualY+offsetY)*scaleY;
}

float Graph::virtual_x(float canvasX){
    return (canvasX)/scaleX-offsetX; 
}

float Graph::virtual_y(float canvasY){
    return (height - canvasY)/scaleY-offsetY;
}

void Graph::update_scale(){

    if(width * height <= 0) return;

    offsetX = -view.x0;
    offsetY = -view.y0;
    scaleX = width/(view.x1-view.x0);
    scaleY = height/(view.y1-view.y0);
}
