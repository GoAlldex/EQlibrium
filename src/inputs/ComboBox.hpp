#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"


/** @brief ComboBox
 * Define look and feel combobox
 */

struct LookAndFeelComboBox : juce::LookAndFeel_V4 {
    LookAndFeelComboBox() {
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(25,25,25));
        setColour(juce::ComboBox::textColourId, juce::Colours::transparentBlack);
        setColour(juce::ComboBox::arrowColourId, juce::Colours::white);
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0,255,128));
        setColour(juce::ComboBox::buttonColourId, juce::Colours::black);
    }
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