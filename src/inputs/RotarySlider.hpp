#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"

/**
 * @brief LookAndFeel rotary slider
 * 
 */
struct LookAndFeelRotarySlider : juce::LookAndFeel_V4 {
    void drawRotarySlider(
        juce::Graphics&,
        int x,
        int y,
        int width,
        int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;
};

/**
 * @brief Struct rotary slider
 * 
 */
struct RotarySliderWithLabels : juce::Slider {
    RotarySliderWithLabels(
        juce::RangedAudioParameter& rap,
        const juce::String& unitSuffix
    ) :
    Slider(
        RotaryHorizontalVerticalDrag,
        NoTextBox
    ),
    param(&rap),
    suffix(unitSuffix) {
        setLookAndFeel(&lnf);
    }
    ~RotarySliderWithLabels() {
        setLookAndFeel(nullptr);
    }
    struct LabelPos {
        float pos;
        juce::String label;
    };
    juce::Array<LabelPos> labels;
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;
private:
    LookAndFeelRotarySlider lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};