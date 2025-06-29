#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"
#include "../image/EQImages.hpp"

struct LookAndFeelNormalImageButtons : juce::LookAndFeel_V4 {
    void drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) override;
};

struct NormalImageButtons : juce::ToggleButton {
    NormalImageButtons(imageNames val) {
        imageSet = val;
    };
    void paint(juce::Graphics& g) override;
    imageNames imageSet;
private:
    juce::String imagePath = juce::File::getSpecialLocation (juce::File::SpecialLocationType::currentExecutableFile).getSiblingFile("images").getFullPathName();
};