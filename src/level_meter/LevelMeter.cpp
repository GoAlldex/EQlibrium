#include "LevelMeter.hpp"

/**
 * @brief Construct a new Left Level Meter Component:: Left Level Meter Component object
 * Set update timer for left channel paint
 * @param p 
 */
LeftLevelMeterComponent::LeftLevelMeterComponent(EQlibriumAudioProcessor& p) :
audioProcessor(p) {
    startTimerHz(60);
}

/**
 * @brief Destroy the Left Level Meter Component:: Left Level Meter Component object
 * Remove listener for left channel
 */
LeftLevelMeterComponent::~LeftLevelMeterComponent() {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->removeListener(this);
    }
}

/**
 * @brief Paint VU-Level meter
 * Paint VU-Level meter left
 * @param g 
 */
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

/**
 * @brief Resize method
 * Resize display elements before paint (removable, left)
 */
void LeftLevelMeterComponent::resized() {
    levelRect = getLocalBounds().toFloat();
}

/**
 * @brief Timer callback
 * Get left value from ./main/pluginProcessor.cpp
 * Paint new VU-Level left
 */
void LeftLevelMeterComponent::timerCallback() {
    setLevel(audioProcessor.getRmsValue(0));
    repaint();
}

//==============================================================================

/**
 * @brief Construct a new Right Level Meter Component:: Right Level Meter Component object
 * Set update timer for right channel paint
 * @param p 
 */
RightLevelMeterComponent::RightLevelMeterComponent(EQlibriumAudioProcessor& p) :
audioProcessor(p) {
    startTimerHz(60);
}

/**
 * @brief Destroy the Right Level Meter Component:: Right Level Meter Component object
 * Remove listener for right channel
 */
RightLevelMeterComponent::~RightLevelMeterComponent() {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->removeListener(this);
    }
}

/**
 * @brief Paint VU-Level meter
 * Paint VU-Level meter right
 * @param g 
 */
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

/**
 * @brief Resize method
 * Resize display elements before paint (removable, right)
 */
void RightLevelMeterComponent::resized() {
    levelRect = getLocalBounds().toFloat();
}

/**
 * @brief Timer callback
 * Get right value from ./main/pluginProcessor.cpp
 * Paint new VU-Level right
 */
void RightLevelMeterComponent::timerCallback() {
    setLevel(audioProcessor.getRmsValue(1));
    repaint();
}