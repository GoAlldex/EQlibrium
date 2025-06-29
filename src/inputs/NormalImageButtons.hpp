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
    NormalImageButtons(imageNames val, float width, float height) {
        imageSet = val;
        this->width = width;
        this->height = height;
    };
    void paint(juce::Graphics& g) override;
    int imageSet;
private:
    float width, height;
    juce::String imagePath = juce::File::getSpecialLocation (juce::File::SpecialLocationType::currentExecutableFile).getSiblingFile("images").getFullPathName();
    void getImages();
};