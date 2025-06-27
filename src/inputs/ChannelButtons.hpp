#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"

struct LookAndFeelChannelButtons : juce::LookAndFeel_V4 {
    void drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) override;
};

struct ChannelButtons : juce::ToggleButton {
    void paint(juce::Graphics& g) override;
    juce::String label;
};