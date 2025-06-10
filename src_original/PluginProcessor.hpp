#pragma once

#include "JuceHeader.h"
#include "juce_dsp/juce_dsp.h"
#include "juce_graphics/fonts/harfbuzz/OT/Layout/GSUB/AlternateSet.hh"

//==============================================================================
class AvSynthAudioProcessor final : public juce::AudioProcessor {
    friend class AvSynthAudioProcessorEditor;

  public:
    enum class Parameters { Gain, Frequency, OscType, LowPassFreq, HighPassFreq, NumParameters };
    enum class OscType { Sine, Square, Saw, Triangle, NumTypes };

    struct ChainSettings {
        float gain = 0.25f;
        float frequency = 440.0f;
        OscType oscType = OscType::Sine;
        float LowPassFreq = 20000.0f;
        float HighPassFreq = 20.0f;

        static forcedinline ChainSettings Get(const juce::AudioProcessorValueTreeState &parameters);
    };

  public:
    //==============================================================================
    AvSynthAudioProcessor();
    ~AvSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    void updateAngleDelta(float frequency);

    static float getOscSample(OscType type, double angle);

    void updateHighPassCoefficients(float frequency);
    void updateLowPassCoefficients(float frequency);

  private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  public:
    juce::AudioProcessorValueTreeState parameters{*this, nullptr, "Parameters", createParameterLayout()};
    juce::MidiKeyboardState keyboardState;

  private:
    juce::Random random;
    ChainSettings previousChainSettings;
    juce::AudioSampleBuffer circularBuffer;
    int bufferWritePos = 0;

  private:
    using Filter = juce::dsp::IIR::Filter<float>;

    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter>;
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, CutFilter>;

    MonoChain leftChain, rightChain;

    double currentAngle = 0.0, angleDelta = 0.0;

  private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AvSynthAudioProcessor)
};
