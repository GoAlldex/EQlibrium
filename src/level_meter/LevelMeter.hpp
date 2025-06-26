#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"

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
};

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
};