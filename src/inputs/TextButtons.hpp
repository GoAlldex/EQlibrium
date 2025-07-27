#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"


/** @brief ComboBox
 * Not used ComboBox == ignore
 */

struct LookAndFeelTextButtons : juce::LookAndFeel_V4 {
    void drawComboBox(
        juce::Graphics& g,
        int width,
        int height,
        bool isButtonDown,
        int buttonX,
        int buttonY,
        int buttonW,
        int buttonH,
        juce::ComboBox &) override;
};

struct TextButtons : juce::ComboBox {
    TextButtons(
        juce::RangedAudioParameter& rap,
        const juce::String& unitSuffix) {};
    void paint(juce::Graphics& g) override;
    juce::StringArray label;
};