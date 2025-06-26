#include "LoudnessSlider.hpp"

void LookAndFeelLinearSlider::drawLinearSlider(
    juce::Graphics & g,
    int x,
    int y,
    int width,
    int height,
    float sliderPos,
    float minSliderPos,
    float maxSliderPos,
    juce::Slider::SliderStyle,
    juce::Slider & slider) {
    using namespace juce;
    Colour LinearBGColour = Colour(25,25,25);
    Colour LinearPosColour = Colour(51,51,255);
    auto bounds = Rectangle<float>(x, y, width, height);
    if(auto* rswl = dynamic_cast<LinearSliderWithLabels*>(&slider)) {
        g.setFont(11);
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        g.setColour(LinearBGColour);
        g.fillRect(x+height/2+30, y+5, width-height-strWidth-3-30, height-10);
        g.setColour(LinearPosColour);
        if(sliderPos <= maxSliderPos) {
            auto newSliderPos = x+30+(((width-height-strWidth-3-30)/maxSliderPos)*sliderPos);
            g.fillEllipse(static_cast<int>(newSliderPos), y, height, height);
        }
        g.setColour(Colours::white);
        g.drawFittedText(text, bounds.toNearestInt(), juce::Justification::right, 1);
    }
}

void LinearSliderWithLabels::paint(juce::Graphics &g) {
    using namespace juce;
    auto range = getRange();
    auto sliderBounds = getSliderBounds();
    getLookAndFeel().drawLinearSlider(
        g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        jmap(
            getValue(),
            range.getStart(),
            range.getEnd(),
            0.0,
            1.0
        ),
        0.0,
        1.0,
        juce::Slider::SliderStyle::LinearVertical,
        *this
    );
    Image speaker = ImageFileFormat::loadFrom(File("E:/Studium/Semester 10/Audiovisual Computing/MS3/EQlibrium/images/speaker.png"));
    g.drawImage(speaker, 15, 0, 20, 20, 0, 0, 1000,1000, false);
    Rectangle<float> r;
    auto str = labels[0].label;
    g.setFont(14);
    r.setSize(g.getCurrentFont().getStringWidth(str), 14);
    r.setLeft(sliderBounds.getX());
    r.setTop(-sliderBounds.getHeight()/2-10);
    r.setY(r.getY()+14);
    g.drawFittedText(str, r.toNearestInt(), juce::Justification::left, 1);
}

juce::Rectangle<int> LinearSliderWithLabels::getSliderBounds() const {
    auto bounds = getLocalBounds();
    return bounds;
}

juce::String LinearSliderWithLabels::getDisplayString() const {
    if(auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param)) {
        return choiceParam->getCurrentChoiceName();
    }
    juce::String str = juce::String(getValue()*100);
    str << suffix;
    return str;
}