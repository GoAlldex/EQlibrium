#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.hpp"

struct CustomRotarySlider : juce::Slider {
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox) {

    }
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
    CustomRotarySlider peakFreqSlider,
        peakGainSlider,
        peakQualitySlider,
        lowCutFreqSlider,
        highCutFreqSlider,
        lowCutSlopeSlider,
        highCutSlopeSlider;
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