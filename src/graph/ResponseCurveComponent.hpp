#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"

/**
 * @brief Struct filter graph
 * 
 */
struct ResponseCurveComponent: juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer {
    ResponseCurveComponent(EQlibriumAudioProcessor&, int);
    ~ResponseCurveComponent();
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }
    void timerCallback() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    EQlibriumAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged { false };
    int channel;
    MonoChain monoChain;
    void updateChain();
    juce::Image background;
    juce::Rectangle<int> getRenderArea();
    juce::Rectangle<int> getAnalysisArea();
};