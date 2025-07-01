#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"

/**
 * @brief Struct left filter graph
 * 
 */
struct LeftResponseCurveComponent: juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer {
    LeftResponseCurveComponent(EQlibriumAudioProcessor&);
    ~LeftResponseCurveComponent();
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }
    void timerCallback() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    EQlibriumAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged { false };
    MonoChain monoChain;
    void updateChain();
    juce::Image background;
    juce::Rectangle<int> getRenderArea();
    juce::Rectangle<int> getAnalysisArea();
};

/**
 * @brief Struct right filter graph
 * 
 */
struct RightResponseCurveComponent: juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer {
    RightResponseCurveComponent(EQlibriumAudioProcessor&);
    ~RightResponseCurveComponent();
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }
    void timerCallback() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    EQlibriumAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged { false };
    MonoChain monoChain;
    void updateChain();
    juce::Image background;
    juce::Rectangle<int> getRenderArea();
    juce::Rectangle<int> getAnalysisArea();
};