#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.hpp"
#include "../graph/ResponseCurveComponent.hpp"
#include "../graph/PathProducer.hpp"
#include "../inputs/RotarySlider.hpp"
#include "../level_meter/LevelMeter.hpp"

//==============================================================================
class EQlibriumAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    EQlibriumAudioProcessorEditor (EQlibriumAudioProcessor&);
    ~EQlibriumAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    juce::Rectangle<int> window_full_rect;
    juce::Rectangle<int> window_micro_rect;
    juce::Rectangle<int> window_settings_rect;
    juce::Rectangle<int> window_filter_rect;
    juce::Rectangle<int> window_filter_left_rect;
    juce::Rectangle<int> window_filter_right_rect;
    juce::Rectangle<int> window_analyser_rect;
    juce::Rectangle<int> window_analyser_left_rect;
    juce::Rectangle<int> window_analyser_left_filter_rect;
    juce::Rectangle<int> window_analyser_left_freq_rect;
    juce::Rectangle<int> window_analyser_right_rect;
    juce::Rectangle<int> window_analyser_right_filter_rect;
    juce::Rectangle<int> window_analyser_right_freq_rect;
    juce::Rectangle<int> window_vumeter_rect;
    juce::Rectangle<int> window_vumeter_left_rect;
    juce::Rectangle<int> window_vumeter_right_rect;
    juce::Rectangle<int> peakL;
    juce::Rectangle<int> peakR;
    juce::Rectangle<int> highCutL;
    juce::Rectangle<int> highCutR;
    juce::Rectangle<int> lowCutL;
    juce::Rectangle<int> lowCutR;
private:
    EQlibriumAudioProcessor& audioProcessor;
    RotarySliderWithLabels peakFreqSliderLeft,
        peakGainSliderLeft,
        peakQualitySliderLeft,
        lowCutFreqSliderLeft,
        highCutFreqSliderLeft,
        lowCutSlopeSliderLeft,
        highCutSlopeSliderLeft,
        peakFreqSliderRight,
        peakGainSliderRight,
        peakQualitySliderRight,
        lowCutFreqSliderRight,
        highCutFreqSliderRight,
        lowCutSlopeSliderRight,
        highCutSlopeSliderRight;
    LeftResponseCurveComponent filterLeft;
    RightResponseCurveComponent filterRight;
    LeftPathProducerComponent freqLeft;
    RightPathProducerComponent freqRight;
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    Attachment leftPeakFreqSliderAttachment,
        leftPeakGainSliderAttachment,
        leftPeakQualitySliderAttachment,
        leftLowCutFreqSliderAttachment,
        leftHighCutFreqSliderAttachment,
        leftLowCutSlopeSliderAttachment,
        leftHighCutSlopeSliderAttachment,
        rightPeakFreqSliderAttachment,
        rightPeakGainSliderAttachment,
        rightPeakQualitySliderAttachment,
        rightLowCutFreqSliderAttachment,
        rightHighCutFreqSliderAttachment,
        rightLowCutSlopeSliderAttachment,
        rightHighCutSlopeSliderAttachment;
    LeftLevelMeterComponent levelMeterLeft;
    RightLevelMeterComponent levelMeterRight;
    std::vector<Component*> getComps();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQlibriumAudioProcessorEditor)
};