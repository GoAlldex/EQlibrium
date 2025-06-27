#pragma once

#include <JuceHeader.h>
#include <array>

template<typename T>
struct Fifo {
    void prepare(int numChannel, int numSamples) {
        static_assert(std::is_same_v<T, juce::AudioBuffer<float>>, "prepare(numChannel, numSamples) should only be used when the Fifo is holding juce::AudioBuffer<float>)");
        for(auto& buffer : buffers) {
            buffer.setSize(
                numChannel,
                numSamples,
                false,
                true,
                true
            );
            buffer.clear();
        }
    }

    void prepare(size_t numElements) {
        static_assert(std::is_same_v<T, std::vector<float>>, "prepare(numElements) should only be used when the Fifo is holding std::vector<float>)");
        for(auto& buffer : buffers) {
            buffer.clear();
            buffer.resize(numElements, 0);
        }
    }

    bool push(const T& t) {
        auto write = fifo.write(1);
        if(write.blockSize1 > 0) {
            buffers[write.startIndex1] = t;
            return true;
        }
        return false;
    }

    bool pull(T& t) {
        auto read = fifo.read(1);
        if(read.blockSize1 > 0) {
            t = buffers[read.startIndex1];
            return true;
        }
        return false;
    }

    int getNumAvailableForReading() const {
        return fifo.getNumReady();
    }
private:
    static constexpr int Capacity = 30;
    std::array<T, Capacity> buffers;
    juce::AbstractFifo fifo {Capacity};
};

enum Channel {
    Right,
    Left
};

template<typename BlockType>
struct SingleChannelSampleFifo {
    SingleChannelSampleFifo(Channel ch) : channelToUse(ch) {
        prepared.set(false);
    }

    void update(const BlockType& buffer) {
        jassert(prepared.get());
        jassert(buffer.getNumChannels() > channelToUse);
        auto* channelPtr = buffer.getReadPointer(channelToUse);
        for(int i = 0; i < buffer.getNumSamples(); ++i) {
            pushNextSampleIntoFifo(channelPtr[i]);
        }
    }

    void prepare(int bufferSize) {
        prepared.set(false);
        size.set(bufferSize);
        bufferToFill.setSize(
            1,
            bufferSize,
            false,
            true,
            true
        );
        audioBufferFifo.prepare(1, bufferSize);
        fifoIndex = 0;
        prepared.set(true);
    }

    int getNumCompleteBuffersAvailable() const { return audioBufferFifo.getNumAvailableForReading(); }
    bool isPrepared() const { return prepared.get(); };
    int getSize() const { return size.get(); }
    bool getAudioBuffer(BlockType& buf) { return audioBufferFifo.pull(buf); }
private:
    Channel channelToUse;
    int fifoIndex = 0;
    Fifo<BlockType> audioBufferFifo;
    BlockType bufferToFill;
    juce::Atomic<bool> prepared = false;
    juce::Atomic<int> size = 0;

    void pushNextSampleIntoFifo(float sample) {
        if(fifoIndex == bufferToFill.getNumSamples()) {
            auto ok = audioBufferFifo.push(bufferToFill);
            juce::ignoreUnused(ok);
            fifoIndex = 0;
        }
        bufferToFill.setSample(0, fifoIndex, sample);
        ++fifoIndex;
    }
};

enum Slope {
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

struct ChainSettings {
    float gainLeft = 1.f;
    float gainRight = 1.f;
    bool channelLeftButton = true;
    bool channelRightButton = true;
    float leftPeakFreq {0}, leftPeakGainInDecibels{0}, leftPeakQuality{1.f}, rightPeakFreq {0}, rightPeakGainInDecibels{0}, rightPeakQuality{1.f};
    float leftLowCutFreq {0}, leftHighCutFreq {0}, rightLowCutFreq {0}, rightHighCutFreq {0};
    Slope leftLowCutSlope {Slope_12}, leftHighCutSlope {Slope_12}, rightLowCutSlope {Slope_12}, rightHighCutSlope {Slope_12};
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);
using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
enum ChainPositions {
    LowCut,
    Peak,
    HighCut
};
using Coefficients = Filter::CoefficientsPtr;
void updateCoefficients(Coefficients& old, const Coefficients& replacements);
Coefficients makeLeftPeakFilter(const ChainSettings& chainSettings, double sampleRate);
Coefficients makeRightPeakFilter(const ChainSettings& chainSettings, double sampleRate);
template<int Index, typename ChainType, typename CoefficientType>
    void update(ChainType& chain, const CoefficientType& coefficients) {
    updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
    chain.template setBypassed<Index>(false);
}
template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& chain, const CoefficientType& coefficients, const Slope& slope) {
    chain.template setBypassed<0>(true);
    chain.template setBypassed<1>(true);
    chain.template setBypassed<2>(true);
    chain.template setBypassed<3>(true);
    switch(slope) {
    case Slope_48:
        update<3>(chain, coefficients);
        break;
    case Slope_36:
        update<2>(chain, coefficients);
        break;
    case Slope_24:
        update<1>(chain, coefficients);
        break;
    case Slope_12:
        update<0>(chain, coefficients);
        break;
    }
}

inline auto makeLeftLowCutFilter(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
        chainSettings.leftLowCutFreq,
        sampleRate,
        2*(chainSettings.leftLowCutSlope+1)
    );
}

inline auto makeLeftHighCutFilter(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
        chainSettings.leftHighCutFreq,
        sampleRate,
        2*(chainSettings.leftHighCutSlope+1)
    );
}

inline auto makeRightLowCutFilter(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
        chainSettings.rightLowCutFreq,
        sampleRate,
        2*(chainSettings.rightLowCutSlope+1)
    );
}

inline auto makeRightHighCutFilter(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
        chainSettings.rightHighCutFreq,
        sampleRate,
        2*(chainSettings.rightHighCutSlope+1)
    );
}

//==============================================================================
class EQlibriumAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    EQlibriumAudioProcessor();
    ~EQlibriumAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
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
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameter", createParameterLayout()};
    using BlockType = juce::AudioBuffer<float>;
    SingleChannelSampleFifo<BlockType> leftChannelFifo { Left };
    SingleChannelSampleFifo<BlockType> rightChannelFifo { Right };
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> playSource;
    void getFile();
    float getRmsValue(const int channel) const;
    void smoothLoudness(juce::AudioBuffer<float>& buffer);
private:
    MonoChain leftChain, rightChain;
    void updatePeakFilter(const ChainSettings& chainSettings);
    void updateLowCutFilters(const ChainSettings& chainSettings);
    void updateHighCutFilters(const ChainSettings& chainSettings);
    void updateFilters();
    juce::LinearSmoothedValue<float> rmsLevelLeft, rmsLevelRight;
    ChainSettings previousChainSettings;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQlibriumAudioProcessor)
};