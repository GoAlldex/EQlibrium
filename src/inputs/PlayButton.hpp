#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"
#include "../image/EQImages.hpp"

/**
 * @brief LookAndFeel play button
 * 
 */
struct LookAndFeelPlayButton : juce::LookAndFeel_V4 {
    void drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) override;
};

/**
 * @brief Struct play button
 * 
 */
struct PlayButton : juce::ToggleButton {
    PlayButton(imageNames play, imageNames pause) {
        imagePlay = play;
        imagePause = pause;
    };
    void paint(juce::Graphics& g) override;
    imageNames imagePlay, imagePause;
};