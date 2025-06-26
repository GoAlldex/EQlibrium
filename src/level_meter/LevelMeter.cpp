#include "LevelMeter.hpp"

LeftLevelMeterComponent::LeftLevelMeterComponent(EQlibriumAudioProcessor& p) :
audioProcessor(p) {
    startTimerHz(60);
}

LeftLevelMeterComponent::~LeftLevelMeterComponent() {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->removeListener(this);
    }
}

void LeftLevelMeterComponent::paint(juce::Graphics& g) {
    using namespace juce;
    auto bounds = getLocalBounds().toFloat();
    g.setColour(Colours::white.withBrightness(0.4f));
    g.fillRect(bounds);
    g.setColour(Colours::white);
    const auto scaleX = jmap(level, -60.f, 6.f, 0.f, static_cast<float>(getWidth()));
    g.fillRect(bounds.removeFromLeft(scaleX));
}

void LeftLevelMeterComponent::resized() {}

void LeftLevelMeterComponent::timerCallback() {
    setLevel(audioProcessor.getRmsValue(0));
    repaint();
}

//==============================================================================

RightLevelMeterComponent::RightLevelMeterComponent(EQlibriumAudioProcessor& p) :
audioProcessor(p) {
    startTimerHz(60);
}

RightLevelMeterComponent::~RightLevelMeterComponent() {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->removeListener(this);
    }
}

void RightLevelMeterComponent::paint(juce::Graphics& g) {
    using namespace juce;
    auto bounds = getLocalBounds().toFloat();
    g.setColour(Colours::white.withBrightness(0.4f));
    g.fillRect(bounds);
    g.setColour(Colours::white);
    const auto scaleX = jmap(level, -60.f, 6.f, 0.f, static_cast<float>(getWidth()));
    g.fillRect(bounds.removeFromLeft(scaleX));
}

void RightLevelMeterComponent::resized() {}

void RightLevelMeterComponent::timerCallback() {
    setLevel(audioProcessor.getRmsValue(1));
    repaint();
}