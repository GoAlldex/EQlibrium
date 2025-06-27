#include "RotarySlider.hpp"

void LookAndFeelRotarySlider::drawRotarySlider(
    juce::Graphics & g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider & slider) {
    using namespace juce;
    Colour rotaryBGColour = Colour(25,25,25);
    Colour rotaryPosColour = Colour(51,51,255);
    auto bounds = Rectangle<float>(x, y, width, height);
    g.setColour(rotaryBGColour);
    g.fillEllipse(bounds);
    g.setColour(Colours::black);
    g.drawEllipse(bounds, 0.1f);
    if(auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)) {
        g.setColour(rotaryPosColour);
        auto center = bounds.getCentre();
        Path p;
        Rectangle<float> r;
        r.setLeft(center.getX()-2);
        r.setRight(center.getX()+2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY()-rswl->getTextHeight()*1.5);
        p.addRectangle(r);
        jassert(rotaryStartAngle < rotaryEndAngle);
        auto sliderAngRad = jmap(
            sliderPosProportional,
            0.f,
            1.f,
            rotaryStartAngle,
            rotaryEndAngle
        );
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
        g.fillPath(p);
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        r.setSize(strWidth+4, rswl->getTextHeight()+2);
        r.setCentre(bounds.getCentre());
        g.setColour(rotaryBGColour);
        g.fillRect(r);
        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), Justification::centred, 1);
    }
}

void RotarySliderWithLabels::paint(juce::Graphics &g) {
    using namespace juce;
    auto startAng = degreesToRadians(180.f+45.f);
    auto endAng = degreesToRadians(180.f-45.f)+MathConstants<float>::twoPi;
    auto range = getRange();
    auto sliderBounds = getSliderBounds();
    getLookAndFeel().drawRotarySlider(
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
        startAng,
        endAng,
        *this
    );
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth()*0.5f;
    g.setColour(Colours::white);
    g.setFont(getTextHeight());
    auto numChoices = labels.size();
    for(int i = 0; i < numChoices; i++) {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        auto ang = jmap(
            pos,
            0.f,
            1.f,
            startAng,
            endAng
        );
        auto c = center.getPointOnCircumference(radius+getTextHeight()*0.5f+1, ang);
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY()+getTextHeight());
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const {
    auto bounds = getLocalBounds();
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    size -= getTextHeight()*2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const {
    if(auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param)) {
        return choiceParam->getCurrentChoiceName();
    }
    juce::String str;
    bool addK = false;
    if(auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param)) {
        float val = getValue();
        if(val > 999.f) {
            val /= 1000.f;
            addK = true;
        }
        str = juce::String(val, (addK ? 2 : 0));
    } else {
        jassertfalse;
    }
    if(suffix.isNotEmpty()) {
        str << " ";
        if(addK) {
            str << "k";
        }
        str << suffix;
    }
    return str;
}