#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"

/**
 * @brief Struct VU-Level meter
 * VU-level meter for left channel
 */
struct LeftLevelMeterComponent: juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer {
    LeftLevelMeterComponent(EQlibriumAudioProcessor&);
    ~LeftLevelMeterComponent();
    void parameterValueChanged(int parameterIndex, float newValue) override { }
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }
    void timerCallback() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setLevel(const float value) { level = value; }
private:
    EQlibriumAudioProcessor& audioProcessor;
    MonoChain monoChain;
    float level = -60.f;
    juce::Rectangle<float> levelRect;
};

/**
 * @brief Struct VU-Level meter
 * VU-level meter for right channel
 */
struct RightLevelMeterComponent: juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer {
    RightLevelMeterComponent(EQlibriumAudioProcessor&);
    ~RightLevelMeterComponent();
    void parameterValueChanged(int parameterIndex, float newValue) override { }
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }
    void timerCallback() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setLevel(const float value) { level = value; }
private:
    EQlibriumAudioProcessor& audioProcessor;
    MonoChain monoChain;
    float level = -60.f;
    juce::Rectangle<float> levelRect;
};