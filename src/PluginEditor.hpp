#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.hpp"

struct LookAndFeel : juce::LookAndFeel_V4 {
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
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};

struct ResponseCurveComponent: juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer {
    ResponseCurveComponent(EQlibriumAudioProcessor&);
    ~ResponseCurveComponent();
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }
    void timerCallback() override;
    void paint(juce::Graphics& g) override;
private:
    EQlibriumAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged { false };
    MonoChain monoChain;
    void updateChain();
};

//==============================================================================
class EQlibriumAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    EQlibriumAudioProcessorEditor (EQlibriumAudioProcessor&);
    ~EQlibriumAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    EQlibriumAudioProcessor& audioProcessor;
    RotarySliderWithLabels peakFreqSlider,
        peakGainSlider,
        peakQualitySlider,
        lowCutFreqSlider,
        highCutFreqSlider,
        lowCutSlopeSlider,
        highCutSlopeSlider;
    ResponseCurveComponent responseCurveComponent;
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    Attachment peakFreqSliderAttachment,
        peakGainSliderAttachment,
        peakQualitySliderAttachment,
        lowCutFreqSliderAttachment,
        highCutFreqSliderAttachment,
        lowCutSlopeSliderAttachment,
        highCutSlopeSliderAttachment;
    std::vector<Component*> getComps();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQlibriumAudioProcessorEditor)
};