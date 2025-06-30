#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"
#include "../image/EQImages.hpp"

struct LookAndFeelPlayButton : juce::LookAndFeel_V4 {
    void drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) override;
};

struct PlayButton : juce::ToggleButton {
    PlayButton(imageNames play, imageNames pause) {
        imagePlay = play;
        imagePause = pause;
    };
    void paint(juce::Graphics& g) override;
    imageNames imagePlay, imagePause;
};