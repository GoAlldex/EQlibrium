#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"

enum imageNames {
    microphone,
    save,
    openFile,
    speaker,
    settings,
    play,
    pause,
    replay
};

struct LookAndFeelNormalImageButtons : juce::LookAndFeel_V4 {
    void drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) override;
};

struct NormalImageButtons : juce::ToggleButton {
    NormalImageButtons(int index) {
        imageSet = index;
        getImages();
    };
    void paint(juce::Graphics& g) override;
    int imageSet;
private:
    juce::String imagePath = juce::File::getSpecialLocation (juce::File::SpecialLocationType::currentExecutableFile).getSiblingFile("images").getFullPathName();
    void getImages();
    juce::Array<juce::String> images;
};