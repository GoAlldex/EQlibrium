#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.hpp"

enum FFTOrder {
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};

template<typename BlockType>
struct FFTDataGenerator {
    void produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData, const float negativeInfinity) {
        const auto fftSize = getFFTSize();
        fftData.assign(fftData.size(), 0);
        auto* readIndex = audioData.getReadPointer(0);
        std::copy(readIndex, readIndex+fftSize, fftData.begin());
        window->multiplyWithWindowingTable(fftData.data(), fftSize);
        forwardFFT->performFrequencyOnlyForwardTransform(fftData.data());
        int numBins = (int) fftSize/2;
        for(int i = 0; i < numBins; i++) {
            fftData[i] /= (float) numBins;
        }
        for(int i = 0; i < numBins; i++) {
            fftData[i] = juce::Decibels::gainToDecibels(fftData[i], negativeInfinity);
        }
        fftDataFifo.push(fftData);
    }

    void changeOrder(FFTOrder newOrder) {
        order = newOrder;
        auto fftSize = getFFTSize();
        forwardFFT = std::make_unique<juce::dsp::FFT>(order);
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize, juce::dsp::WindowingFunction<float>::blackmanHarris);
        fftData.clear();
        fftData.resize(fftSize*2, 0);
        fftDataFifo.prepare(fftData.size());
    }

    int getFFTSize() const { return 1 << order; }
    int getNumAvailableFFTDataBlocks() const { return fftDataFifo.getNumAvailableForReading(); }
    bool getFFTData(BlockType& fftData) { return fftDataFifo.pull(fftData); }
private:
    FFTOrder order;
    BlockType fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    Fifo<BlockType> fftDataFifo;
};

template<typename PathType>
struct AnalyzerPathGenerator {
    void generatePath(
        const std::vector<float>& renderData,
        juce::Rectangle<float> fftBounds,
        int fftSize,
        float binWidth,
        float negativeInfinity
    ) {
        auto top = fftBounds.getY();
        auto bottom = fftBounds.getHeight();
        auto width = fftBounds.getWidth();
        int numBins = (int)fftSize/2;
        PathType p;
        p.preallocateSpace(3*(int)fftBounds.getWidth());
        auto map = [bottom, top, negativeInfinity](float v) {
            return juce::jmap(v, negativeInfinity, 0.f, float(bottom), top);
        };
        auto y = map(renderData[0]);
        jassert(!std::isnan(y) && !std::isinf(y));
        p.startNewSubPath(0, y);
        const int pathResolution = 2;
        for(int binNum = 1; binNum < numBins; binNum += pathResolution) {
            y = map(renderData[binNum]);
            jassert(!std::isnan(y) && !std::isinf(y));
            if(!std::isnan(y) && !std::isinf(y)) {
                auto binFreq = binNum*binWidth;
                auto normalizedBinX = juce::mapFromLog10(binFreq, 20.f, 20000.f);
                int binX = std::floor(normalizedBinX*width);
                p.lineTo(binX,y);
            }
        }
        pathFifo.push(p);
    }

    int getNumPathsAvailable() const { return pathFifo.getNumAvailableForReading(); }
    bool getPath(PathType& path) { return pathFifo.pull(path); }
private:
    Fifo<PathType> pathFifo;
};

struct LookAndFeel : juce::LookAndFeel_V4 {
    void drawRotarySlider(
        juce::Graphics&,
        int x,
        int y,
        int width,
        int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;
};

struct RotarySliderWithLabels : juce::Slider {
    RotarySliderWithLabels(
        juce::RangedAudioParameter& rap,
        const juce::String& unitSuffix
    ) :
    Slider(
        RotaryHorizontalVerticalDrag,
        NoTextBox
    ),
    param(&rap),
    suffix(unitSuffix) {
        setLookAndFeel(&lnf);
    }
    ~RotarySliderWithLabels() {
        setLookAndFeel(nullptr);
    }
    struct LabelPos {
        float pos;
        juce::String label;
    };
    juce::Array<LabelPos> labels;
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;
private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};

struct PathProducer {
    PathProducer(SingleChannelSampleFifo<EQlibriumAudioProcessor::BlockType>& scsf) :
    ChannelFifo(&scsf) {
        ChannelFFTDataGenerator.changeOrder(order4096);
        monoBuffer.setSize(1, ChannelFFTDataGenerator.getFFTSize());
    }
    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    juce::Path getPath() { return ChannelFFTPath; }
private:
    SingleChannelSampleFifo<EQlibriumAudioProcessor::BlockType>* ChannelFifo;
    juce::AudioBuffer<float> monoBuffer;
    FFTDataGenerator<std::vector<float>> ChannelFFTDataGenerator;
    AnalyzerPathGenerator<juce::Path> pathProducer;
    juce::Path ChannelFFTPath;
};

struct ResponseCurveComponent: juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer {
    ResponseCurveComponent(EQlibriumAudioProcessor&);
    ~ResponseCurveComponent();
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }
    void timerCallback() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setCurveComponent(int cc) { curveComponent = cc; }
    void paintLeftFilter(juce::Graphics& g, juce::Rectangle<int> &responseArea, int w, int h, std::vector<double> &mags);
    void paintRightFilter(juce::Graphics& g, juce::Rectangle<int> &responseArea, int w, int h, std::vector<double> &mags);
    void paintLeftFreq(juce::Graphics& g, juce::Rectangle<int> &responseArea, int w, int h, std::vector<double> &mags);
    void paintRightFreq(juce::Graphics& g, juce::Rectangle<int> &responseArea, int w, int h, std::vector<double> &mags);
private:
    EQlibriumAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged { false };
    MonoChain monoChain;
    void updateChain();
    juce::Image background;
    juce::Rectangle<int> getRenderArea();
    juce::Rectangle<int> getAnalysisArea();
    PathProducer leftPathProducer, rightPathProducer;
    int curveComponent;
};

//==============================================================================
class EQlibriumAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    EQlibriumAudioProcessorEditor (EQlibriumAudioProcessor&);
    ~EQlibriumAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    juce::Rectangle<int> window_full_rect;
    juce::Rectangle<int> window_micro_rect;
    juce::Rectangle<int> window_settings_rect;
    juce::Rectangle<int> window_filter_rect;
    juce::Rectangle<int> window_filter_left_rect;
    juce::Rectangle<int> window_filter_right_rect;
    juce::Rectangle<int> window_analyser_rect;
    juce::Rectangle<int> window_analyser_left_rect;
    juce::Rectangle<int> window_analyser_left_filter_rect;
    juce::Rectangle<int> window_analyser_left_freq_rect;
    juce::Rectangle<int> window_analyser_right_rect;
    juce::Rectangle<int> window_analyser_right_filter_rect;
    juce::Rectangle<int> window_analyser_right_freq_rect;
    juce::Rectangle<int> window_vumeter_rect;
    juce::Rectangle<int> window_vumeter_left_rect;
    juce::Rectangle<int> window_vumeter_right_rect;
private:
    EQlibriumAudioProcessor& audioProcessor;
    RotarySliderWithLabels peakFreqSliderLeft,
        peakGainSliderLeft,
        peakQualitySliderLeft,
        lowCutFreqSliderLeft,
        highCutFreqSliderLeft,
        lowCutSlopeSliderLeft,
        highCutSlopeSliderLeft;
    ResponseCurveComponent filterLeft, freqLeft;
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