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
{
    getFile();
}

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

void EQlibriumAudioProcessor::getFile()
{
    formatManager.registerBasicFormats();
    auto file = juce::File {"E:/Studium/Semester 10/Audiovisual Computing/MS3/Test_Music/Repiet - All I Need.mp3"};
    auto* reader = formatManager.createReaderFor(file);
    if(reader != nullptr) {
        playSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
        playSource->setLooping(true);
    } else {
        juce::JUCEApplicationBase::quit();
    }
}

//==============================================================================
void EQlibriumAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    previousChainSettings = getChainSettings(apvts);
    juce::dsp::ProcessSpec spec {
        spec.sampleRate = sampleRate,
        spec.maximumBlockSize = samplesPerBlock,
        spec.numChannels = 1
    };
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    updateFilters();
    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);
    rmsLevelLeft.reset(sampleRate, 0.2);
    rmsLevelLeft.setCurrentAndTargetValue(-100.f);
    rmsLevelRight.reset(sampleRate, 0.2);
    rmsLevelRight.setCurrentAndTargetValue(-100.f);
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

void EQlibriumAudioProcessor::smoothLoudness(juce::AudioBuffer<float>& buffer) {
    if (juce::approximatelyEqual(getChainSettings(apvts).gainLeft, previousChainSettings.gainLeft)) {
        buffer.applyGain(0, 0, buffer.getNumSamples(), previousChainSettings.gainLeft);
    } else {
        buffer.applyGainRamp(0, 0, buffer.getNumSamples(), previousChainSettings.gainLeft, getChainSettings(apvts).gainLeft);
    }
    if (juce::approximatelyEqual(getChainSettings(apvts).gainRight, previousChainSettings.gainRight)) {
        buffer.applyGain(1, 0, buffer.getNumSamples(), previousChainSettings.gainRight);
    } else {
        buffer.applyGainRamp(1, 0, buffer.getNumSamples(), previousChainSettings.gainRight, getChainSettings(apvts).gainRight);
    }
    previousChainSettings = getChainSettings(apvts);
}

void EQlibriumAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    updateFilters();
    juce::dsp::AudioBlock<float> block(buffer);
    if(playSource) {
        juce::AudioSourceChannelInfo info(buffer);
        playSource->getNextAudioBlock(info);
    }
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    leftChain.process(leftContext);
    rightChain.process(rightContext);
    leftChannelFifo.update(buffer);
    rightChannelFifo.update(buffer);
    smoothLoudness(buffer);
    rmsLevelLeft.skip(buffer.getNumSamples());
    rmsLevelRight.skip(buffer.getNumSamples());
    {
        const auto value = juce::Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
        if(value < rmsLevelLeft.getCurrentValue()) {
            rmsLevelLeft.setTargetValue(value);
        } else {
            rmsLevelLeft.setCurrentAndTargetValue(value);
        }
    }
    {
        const auto value = juce::Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));
        if(value < rmsLevelRight.getCurrentValue()) {
            rmsLevelRight.setTargetValue(value);
        } else {
            rmsLevelRight.setCurrentAndTargetValue(value);
        }
    }
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
    settings.leftLowCutFreq = apvts.getRawParameterValue("Left LowCut Freq")->load();
    settings.leftHighCutFreq = apvts.getRawParameterValue("Left HighCut Freq")->load();
    settings.leftPeakFreq = apvts.getRawParameterValue("Left Peak Freq")->load();
    settings.leftPeakGainInDecibels = apvts.getRawParameterValue("Left Peak Gain")->load();
    settings.leftPeakQuality = apvts.getRawParameterValue("Left Peak Quality")->load();
    settings.leftLowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("Left LowCut Slope")->load());
    settings.leftHighCutSlope = static_cast<Slope>(apvts.getRawParameterValue("Left HighCut Slope")->load());
    settings.rightLowCutFreq = apvts.getRawParameterValue("Right LowCut Freq")->load();
    settings.rightHighCutFreq = apvts.getRawParameterValue("Right HighCut Freq")->load();
    settings.rightPeakFreq = apvts.getRawParameterValue("Right Peak Freq")->load();
    settings.rightPeakGainInDecibels = apvts.getRawParameterValue("Right Peak Gain")->load();
    settings.rightPeakQuality = apvts.getRawParameterValue("Right Peak Quality")->load();
    settings.rightLowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("Right LowCut Slope")->load());
    settings.rightHighCutSlope = static_cast<Slope>(apvts.getRawParameterValue("Right HighCut Slope")->load());
    settings.gainLeft = apvts.getRawParameterValue("Left Gain Slider")->load();
    settings.gainRight = apvts.getRawParameterValue("Right Gain Slider")->load();
    settings.channelLeftButton = apvts.getRawParameterValue("Left Channel Button")->load();
    settings.channelRightButton = apvts.getRawParameterValue("Right Channel Button")->load();
    return settings;
}

Coefficients makeLeftPeakFilter(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate,
        chainSettings.leftPeakFreq,
        chainSettings.leftPeakQuality,
        juce::Decibels::decibelsToGain(
            chainSettings.leftPeakGainInDecibels
        )
    );
}

Coefficients makeRightPeakFilter(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate,
        chainSettings.rightPeakFreq,
        chainSettings.rightPeakQuality,
        juce::Decibels::decibelsToGain(
            chainSettings.rightPeakGainInDecibels
        )
    );
}

void EQlibriumAudioProcessor::updatePeakFilter(const ChainSettings &chainSettings) {
    auto leftPeakCoefficients = makeLeftPeakFilter(chainSettings, getSampleRate());
    auto rightPeakCoefficients = makeRightPeakFilter(chainSettings, getSampleRate());
    updateCoefficients(leftChain.get<Peak>().coefficients, leftPeakCoefficients);
    updateCoefficients(rightChain.get<Peak>().coefficients, rightPeakCoefficients);
}

void updateCoefficients(Coefficients &old, const Coefficients &replacements) {
    *old = *replacements;
}

void EQlibriumAudioProcessor::updateLowCutFilters(const ChainSettings& chainSettings) {
    auto leftLowCutCoefficients = makeLeftLowCutFilter(chainSettings, getSampleRate());
    auto rightLowCutCoefficients = makeRightLowCutFilter(chainSettings, getSampleRate());
    auto& leftLowCut = leftChain.get<LowCut>();
    auto& rightLowCut = rightChain.get<LowCut>();
    updateCutFilter(leftLowCut, leftLowCutCoefficients, chainSettings.leftLowCutSlope);
    updateCutFilter(rightLowCut, rightLowCutCoefficients, chainSettings.rightLowCutSlope);
}

void EQlibriumAudioProcessor::updateHighCutFilters(const ChainSettings& chainSettings) {
    auto leftHighCutCoefficients = makeLeftHighCutFilter(chainSettings, getSampleRate());
    auto rightHighCutCoefficients = makeRightHighCutFilter(chainSettings, getSampleRate());
    auto& leftHighCut = leftChain.get<HighCut>();
    auto& rightHighCut = rightChain.get<HighCut>();
    updateCutFilter(leftHighCut, leftHighCutCoefficients, chainSettings.leftHighCutSlope);
    updateCutFilter(rightHighCut, rightHighCutCoefficients, chainSettings.rightHighCutSlope);
}

void EQlibriumAudioProcessor::updateFilters() {
    auto chainSettings = getChainSettings(apvts);
    updatePeakFilter(chainSettings);
    updateLowCutFilters(chainSettings);
    updateHighCutFilters(chainSettings);
}

float EQlibriumAudioProcessor::getRmsValue(const int channel) const {
    jassert(channel == 0 || channel == 1);
    if(channel == 0) {
        return rmsLevelLeft.getCurrentValue();
    } else if(channel == 1) {
        return rmsLevelRight.getCurrentValue();
    }
    return 0.f;
}

juce::AudioProcessorValueTreeState::ParameterLayout EQlibriumAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Left LowCut Freq",
        "Left LowCut Freq",
        juce::NormalisableRange<float>(20.f,20000.f,1.f,0.25f),
        20.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Left HighCut Freq",
        "Left HighCut Freq",
        juce::NormalisableRange<float>(20.f,20000.f,1.f,0.25f),
        20000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Left Peak Freq",
        "Left Peak Freq",
        juce::NormalisableRange<float>(20.f,20000.f,1.f,0.25f),
        750.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Left Peak Gain",
        "Left Peak Gain",
        juce::NormalisableRange<float>(-24.f,24.f,0.5f,1.f),
        0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Left Peak Quality",
        "Left Peak Quality",
        juce::NormalisableRange<float>(0.1f,10.f,0.05f,1.f),
        1.f));
    juce::StringArray stringArray;
    for(int i = 0; i < 4; i++) {
        juce::String str;
        str << (12+i*12);
        str << "db/Oct";
        stringArray.add(str);
    }
    layout.add(std::make_unique<juce::AudioParameterChoice>("Left LowCut Slope", "Left LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("Left HighCut Slope", "Left HighCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Right LowCut Freq",
        "Right LowCut Freq",
        juce::NormalisableRange<float>(20.f,20000.f,1.f,0.25f),
        20.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Right HighCut Freq",
        "Right HighCut Freq",
        juce::NormalisableRange<float>(20.f,20000.f,1.f,0.25f),
        20000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Right Peak Freq",
        "Right Peak Freq",
        juce::NormalisableRange<float>(20.f,20000.f,1.f,0.25f),
        750.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Right Peak Gain",
        "Right Peak Gain",
        juce::NormalisableRange<float>(-24.f,24.f,0.5f,1.f),
        0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Right Peak Quality",
        "Right Peak Quality",
        juce::NormalisableRange<float>(0.1f,10.f,0.05f,1.f),
        1.f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("Right LowCut Slope", "Right LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("Right HighCut Slope", "Right HighCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Left Gain Slider",
        "Left Gain Slider",
        juce::NormalisableRange<float>(0.f,1.f,0.05f,2.f),
        1.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Right Gain Slider",
        "Right Gain Slider",
        juce::NormalisableRange<float>(0.f,1.f,0.05f,2.f),
        1.f));
    auto attributes = juce::AudioParameterBoolAttributes();
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "Left Channel Button",
        "Left Channel Button",
        true,
        attributes));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "Right Channel Button",
        "Right Channel Button",
        true,
        attributes));
    return layout;
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new EQlibriumAudioProcessor(); }