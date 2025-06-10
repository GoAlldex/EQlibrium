#pragma once

#include "PluginProcessor.hpp"
#include "WaveformComponent.hpp"

//==============================================================================
class AvSynthAudioProcessorEditor final : public juce::AudioProcessorEditor {
  public:
    explicit AvSynthAudioProcessorEditor(AvSynthAudioProcessor &);
    ~AvSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

  private:
    std::vector<juce::Component *> GetComps();

  private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AvSynthAudioProcessor &processorRef;

    juce::Slider gainSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment;
    juce::Slider frequencySlider;
    juce::AudioProcessorValueTreeState::SliderAttachment frequencyAttachment;
    juce::ComboBox oscTypeComboBox;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment oscTypeAttachment;
    juce::Slider lowCutFreqSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment lowCutFreqAttachment;
    juce::Slider highCutFreqSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment highCutFreqAttachment;
    juce::MidiKeyboardComponent keyboardComponent;
    WaveformComponent waveformComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AvSynthAudioProcessorEditor)
};
