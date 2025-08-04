#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"
#include "../image/EQImages.hpp"

/**
 * @brief LookAndFeel image buttons
 * 
 */
struct LookAndFeelNormalImageButtons : juce::LookAndFeel_V4 {
    void drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) override;
};

/**
 * @brief Struct image buttons
 * 
 */
struct NormalImageButtons : juce::ToggleButton {
    NormalImageButtons(imageNames val) {
        imageSet = val;
    };
    void paint(juce::Graphics& g) override;
    imageNames imageSet;
};