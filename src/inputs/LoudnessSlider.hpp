#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"

struct LookAndFeelLinearSlider : juce::LookAndFeel_V4 {
    void drawLinearSlider(
        juce::Graphics&,
        int x,
        int y,
        int width,
        int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        juce::Slider::SliderStyle,
        juce::Slider&) override;
};

struct LinearSliderWithLabels : juce::Slider {
    LinearSliderWithLabels(
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
    ~LinearSliderWithLabels() {
        setLookAndFeel(nullptr);
    }
    struct LabelPos {
        float pos;
        juce::String label;
    };
    juce::Array<LabelPos> labels;
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    juce::String getDisplayString() const;
private:
    LookAndFeelLinearSlider lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};