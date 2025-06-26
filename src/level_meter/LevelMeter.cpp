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
    auto bounds = levelRect;
    g.fillRect(bounds);
    ColourGradient gradient {
          Colour(6,250,126),
          bounds.getBottomLeft(),
          Colour(254,51,51),
          bounds.getBottomRight(),
          false
    };
    gradient.addColour(0.5, Colour(Colours::yellow));
    g.setGradientFill(gradient);
    g.fillRect(bounds);
    g.setColour(Colour(51,51,255).withAlpha(0.8f));
    const auto scaleX = jmap(level, -60.f, 6.f, 0.f, static_cast<float>(getWidth()));
    g.fillRect(bounds.removeFromLeft(scaleX));
}

void LeftLevelMeterComponent::resized() {
    levelRect = getLocalBounds().toFloat();
    levelRect.removeFromBottom(60);
}

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
    auto bounds = levelRect;
    g.fillRect(bounds);
    ColourGradient gradient {
        Colour(6,250,126),
        bounds.getBottomLeft(),
        Colour(254,51,51),
        bounds.getBottomRight(),
        false
    };
    gradient.addColour(0.5, Colour(Colours::yellow));
    g.setGradientFill(gradient);
    g.fillRect(bounds);
    g.setColour(Colour(51,51,255).withAlpha(0.8f));
    const auto scaleX = jmap(level, -60.f, 6.f, 0.f, static_cast<float>(getWidth()));
    g.fillRect(bounds.removeFromLeft(scaleX));
}

void RightLevelMeterComponent::resized() {
    levelRect = getLocalBounds().toFloat();
    levelRect.removeFromBottom(60);
}

void RightLevelMeterComponent::timerCallback() {
    setLevel(audioProcessor.getRmsValue(1));
    repaint();
}