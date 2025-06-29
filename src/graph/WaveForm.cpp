#include "WaveForm.hpp"

WaveForm::WaveForm(EQlibriumAudioProcessor& p) :
audioProcessor(p) {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->addListener(this);
    }
    thumbnail = p.getThumbnail();
    startTimerHz(30);
}

WaveForm::~WaveForm() {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->removeListener(this);
    }
}

void WaveForm::timerCallback() {
    repaint();
}

void WaveForm::paint(juce::Graphics& g) {
    using namespace juce;
    auto bounds = getBounds();
    bounds.setLeft(0);
    bounds.setTop(0);
    g.setColour(Colour(25,25,25));
    g.fillRect(bounds);
    bounds.removeFromBottom(3);
    bounds.removeFromTop(3);
    bounds.removeFromLeft(3);
    bounds.removeFromRight(3);
    g.setColour(Colour(51,51,255));
    thumbnail->drawChannels(g,
        bounds,
        0.0,
        thumbnail->getTotalLength(),
        1.f);
}

void WaveForm::resized() {
    using namespace juce;

}

void WaveForm::parameterValueChanged(int parameterIndex, float newValue) {
    parametersChanged.set(true);
}