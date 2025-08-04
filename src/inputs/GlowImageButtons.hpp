#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"
#include "../image/EQImages.hpp"

/**
 * @brief LookAndFeel glow image button
 * 
 */
struct LookAndFeelGlowButton : juce::LookAndFeel_V4 {
    void drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) override;
};

/**
 * @brief Struct glow image button
 * 
 */
struct GlowImageButtons : juce::ToggleButton {
    GlowImageButtons(imageNames img) {
        image = img;
    };
    void paint(juce::Graphics& g) override;
    imageNames image;
};