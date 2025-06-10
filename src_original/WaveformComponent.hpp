#pragma once

#include "JuceHeader.h"

class WaveformComponent : public juce::Component, public juce::Timer {
  public:
    WaveformComponent(juce::AudioSampleBuffer &bufferRef, int &writePosRef);

    void paint(juce::Graphics &g) override;

  private:
    void timerCallback() override;

    void drawWaveform(juce::Graphics &g) const;

    juce::AudioSampleBuffer &buffer;
    int &writePos;
};