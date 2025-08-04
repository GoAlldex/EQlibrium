#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"

/**
 * @brief Struct VU-Level meter
 * VU-level meter
 */
struct LevelMeterComponent: juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer {
    LevelMeterComponent(EQlibriumAudioProcessor&, int);
    ~LevelMeterComponent();
    void parameterValueChanged(int parameterIndex, float newValue) override { }
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }
    void timerCallback() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setLevel(const float value) { level = value; }
private:
    EQlibriumAudioProcessor& audioProcessor;
    int channel;
    MonoChain monoChain;
    float level = -60.f;
    juce::Rectangle<float> levelRect;
};