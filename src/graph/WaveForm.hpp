#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"

/**
 * @brief Struct waveform
 * 
 */
struct WaveForm : juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer {
    WaveForm(EQlibriumAudioProcessor&);
    ~WaveForm();
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }
    void timerCallback() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    EQlibriumAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged { false };
    MonoChain monoChain;
    juce::AudioThumbnail* thumbnail;
};