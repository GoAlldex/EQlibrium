#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"
#include "../image/EQImages.hpp"

struct LookAndFeelReplayButton : juce::LookAndFeel_V4 {
    void drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) override;
};

struct ReplayButton : juce::ToggleButton {
    ReplayButton(imageNames replay) {
        image = replay;
    };
    void paint(juce::Graphics& g) override;
    imageNames image;
};