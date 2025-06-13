#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

//==============================================================================
EQlibriumAudioProcessor::EQlibriumAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{}

EQlibriumAudioProcessor::~EQlibriumAudioProcessor() {}

//==============================================================================
const juce::String EQlibriumAudioProcessor::getName() const { return JucePlugin_Name; }

bool EQlibriumAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EQlibriumAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EQlibriumAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EQlibriumAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int EQlibriumAudioProcessor::getNumPrograms() { return 1; }
int EQlibriumAudioProcessor::getCurrentProgram() { return 0; }
void EQlibriumAudioProcessor::setCurrentProgram (int index) {}
const juce::String EQlibriumAudioProcessor::getProgramName (int index) { return {}; }
void EQlibriumAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

//==============================================================================
void EQlibriumAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    juce::dsp::ProcessSpec spec {
        spec.sampleRate = sampleRate,
        spec.maximumBlockSize = samplesPerBlock,
        spec.numChannels = 1
    };
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    updateFilters();
}

void EQlibriumAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EQlibriumAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void EQlibriumAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    updateFilters();
    juce::dsp::AudioBlock<float> block(buffer);
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool EQlibriumAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* EQlibriumAudioProcessor::createEditor() { return new EQlibriumAudioProcessorEditor(*this); }

//==============================================================================
void EQlibriumAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void EQlibriumAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if(tree.isValid()) {
        apvts.replaceState(tree);
        updateFilters();
    }
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts) {
    ChainSettings settings;
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());
    return settings;
}

Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate,
        chainSettings.peakFreq,
        chainSettings.peakQuality,
        juce::Decibels::decibelsToGain(
            chainSettings.peakGainInDecibels
        )
    );
}

void EQlibriumAudioProcessor::updatePeakFilter(const ChainSettings &chainSettings) {
    auto peakCoefficients = makePeakFilter(chainSettings, getSampleRate());
    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
}

void updateCoefficients(Coefficients &old, const Coefficients &replacements) {
    *old = *replacements;
}

void EQlibriumAudioProcessor::updateLowCutFilters(const ChainSettings& chainSettings) {
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
        chainSettings.lowCutFreq,
        getSampleRate(),
        2*(chainSettings.lowCutSlope+1)
    );
    auto& leftLowCut = leftChain.get<LowCut>();
    auto& rightLowCut = rightChain.get<LowCut>();
    updateCutFilter(leftLowCut, lowCutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(rightLowCut, lowCutCoefficients, chainSettings.lowCutSlope);
}

void EQlibriumAudioProcessor::updateHighCutFilters(const ChainSettings& chainSettings) {
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
        chainSettings.highCutFreq,
        getSampleRate(),
        2*(chainSettings.highCutSlope+1)
    );
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
    updateCutFilter(leftHighCut, highCutCoefficients, chainSettings.highCutSlope);
    updateCutFilter(rightHighCut, highCutCoefficients, chainSettings.highCutSlope);
}

void EQlibriumAudioProcessor::updateFilters() {
    auto chainSettings = getChainSettings(apvts);
    updatePeakFilter(chainSettings);
    updateLowCutFilters(chainSettings);
    updateHighCutFilters(chainSettings);
}

juce::AudioProcessorValueTreeState::ParameterLayout EQlibriumAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "LowCut Freq",
        "LowCut Freq",
        juce::NormalisableRange<float>(20.f,20000.f,1.f,0.25f),
        20.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "HighCut Freq",
        "HighCut Freq",
        juce::NormalisableRange<float>(20.f,20000.f,1.f,0.25f),
        20000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak Freq",
        "Peak Freq",
        juce::NormalisableRange<float>(20.f,20000.f,1.f,0.25f),
        750.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak Gain",
        "Peak Gain",
        juce::NormalisableRange<float>(-24.f,24.f,0.5f,1.f),
        0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak Quality",
        "Peak Quality",
        juce::NormalisableRange<float>(0.1f,10.f,0.05f,1.f),
        1.f));
    juce::StringArray stringArray;
    for(int i = 0; i < 4; i++) {
        juce::String str;
        str << (12+i*12);
        str << "db/Oct";
        stringArray.add(str);
    }
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));
    return layout;
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new EQlibriumAudioProcessor(); }