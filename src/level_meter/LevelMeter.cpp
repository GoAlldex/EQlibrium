#include "LevelMeter.hpp"

/**
 * @brief Construct a new Level Meter Component:: Level Meter Component object
 * Set update timer for channel paint
 * @param p 
 */
LevelMeterComponent::LevelMeterComponent(EQlibriumAudioProcessor& p, int chval) :
audioProcessor(p) {
    channel = chval;
    startTimerHz(60);
}

/**
 * @brief Destroy the Level Meter Component:: Level Meter Component object
 * Remove listener
 */
LevelMeterComponent::~LevelMeterComponent() {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->removeListener(this);
    }
}

/**
 * @brief Paint VU-Level meter
 * Paint VU-Level meter
 * @param g 
 */
void LevelMeterComponent::paint(juce::Graphics& g) {
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
 * Resize display elements before paint (removable)
 */
void LevelMeterComponent::resized() {
    levelRect = getLocalBounds().toFloat();
}

/**
 * @brief Timer callback
 * Get value from ./main/pluginProcessor.cpp
 * Paint new VU-Level
 */
void LevelMeterComponent::timerCallback() {
    setLevel(audioProcessor.getRmsValue(channel == 0 ? 0 : 1));
    repaint();
}