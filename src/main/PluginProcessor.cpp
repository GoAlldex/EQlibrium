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
                       ), thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache)
#endif
{ }

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

/**
 * @brief File chooser
 * Open default file Handler
 * Read file data
 * Load data for waveform display
 * Set audio loop when replay button is on active state
 */
void EQlibriumAudioProcessor::getFile() {
    formatManager.registerBasicFormats();
    juce::FileChooser chooser("Datei ausw\u00e4hlen", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav; *.mp3");
    if(chooser.browseForFileToOpen()) {
        auto file = chooser.getResult();
        juce::AudioFormatReader* reader = formatManager.createReaderFor(file);
        if (reader != nullptr) {
            thumbnail.setSource(new juce::FileInputSource(file));
            playSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            if(getChainSettings(apvts).replayButton) {
                playSource->setLooping(true);
            }
        }
    }
}

/**
 * @brief Set loop
 * Set loop for audio data (if audio data exists)
 */
void EQlibriumAudioProcessor::loop() {
    if(playSource) {
        playSource->isLooping() ? playSource->setLooping(false) : playSource->setLooping(true);
    }
}

/**
 * @brief Getter for waveform display
 * Used in ./graph/WaveForm.cpp for display waveform left and right channel
 * @return juce::AudioThumbnail* 
 */
juce::AudioThumbnail* EQlibriumAudioProcessor::getThumbnail() {
    return &thumbnail;
}

//==============================================================================
/**
 * @brief Prepare to play
 * Set filters
 * Set VU-level with delay
 * @param sampleRate 
 * @param samplesPerBlock 
 */
void EQlibriumAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    previousChainSettings = getChainSettings(apvts);
    juce::dsp::ProcessSpec spec {
        spec.sampleRate = sampleRate,
        spec.maximumBlockSize = samplesPerBlock,
        spec.numChannels = 1
    };
    leftChain.prepare(spec);
    leftChannelFifo.prepare(samplesPerBlock);
    rmsLevelLeft.reset(sampleRate, 0.2);
    rmsLevelLeft.setCurrentAndTargetValue(-100.f);
    rightChain.prepare(spec);
    rightChannelFifo.prepare(samplesPerBlock);
    rmsLevelRight.reset(sampleRate, 0.2);
    rmsLevelRight.setCurrentAndTargetValue(-100.f);
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

/**
 * @brief Smooth loudness
 * Slowly decrease/increase of gain from old to new gain value
 * @param buffer 
 */
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
}

/**
 * @brief Record voice
 * Record buffer data to file (or stop record)
 * @param buffer 
 */
void EQlibriumAudioProcessor::recordVoice(juce::AudioBuffer<float>& buffer) {
    if(getChainSettings(apvts).recordButton) {
        writeFile = true;
        if(!file.exists()) {
            file.create();
            writer = writerformat.createWriterFor(new juce::FileOutputStream(file), getSampleRate(), getTotalNumOutputChannels(), 24, {}, 0);
        }
        if(writer != nullptr) {
            writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
        }
    } else if(writeFile) {
        writeFile = false;
        writer->flush();
        //writer.reset(writerformat.createWriterFor(new juce::FileOutputStream(file), getSampleRate(), getTotalNumOutputChannels(), 24, {}, 0));
    }
}

/**
 * @brief Process block
 * Play audio data (when play == true and audio data exists)
 * Set filters lowcut, highcut, notch
 * Set values for VU-level display
 * Set voice record after button activation
 * @param buffer 
 * @param midiMessages 
 */
void EQlibriumAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    updateFilters();
    juce::dsp::AudioBlock<float> block(buffer);
    if(playSource && getChainSettings(apvts).playButton) {
        juce::AudioSourceChannelInfo info(buffer);
        playSource->getNextAudioBlock(info);
        recordVoice(buffer);
    }
    auto leftBlock = getChainSettings(apvts).channelLeftButton ? block.getSingleChannelBlock(0) : block.getSingleChannelBlock(0).clear();
    auto rightBlock = getChainSettings(apvts).channelRightButton ? block.getSingleChannelBlock(1) : block.getSingleChannelBlock(1).clear();
    smoothLoudness(buffer);
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    leftChain.process(leftContext);
    leftChannelFifo.update(buffer);
    rmsLevelLeft.skip(buffer.getNumSamples());
    {
        const auto value = juce::Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
        if(value < rmsLevelLeft.getCurrentValue()) {
            rmsLevelLeft.setTargetValue(value);
        } else {
            rmsLevelLeft.setCurrentAndTargetValue(value);
        }
    }
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    rightChain.process(rightContext);
    rightChannelFifo.update(buffer);
    rmsLevelRight.skip(buffer.getNumSamples());
    {
        const auto value = juce::Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));
        if(value < rmsLevelRight.getCurrentValue()) {
            rmsLevelRight.setTargetValue(value);
        } else {
            rmsLevelRight.setCurrentAndTargetValue(value);
        }
    }
    recordVoice(buffer);
    previousChainSettings = getChainSettings(apvts);
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

/**
 * @brief Get the Chain Settings object
 * Load previous settings into chainsettings
 * @param apvts 
 * @return ChainSettings 
 */
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
    settings.playButton = apvts.getRawParameterValue("Play Button")->load();
    settings.replayButton = apvts.getRawParameterValue("Replay Button")->load();
    settings.recordButton = apvts.getRawParameterValue("Microphone Button")->load();
    return settings;
}

/**
 * @brief Peak filter
 * Peak filter for left channel
 * @param chainSettings 
 * @param sampleRate 
 * @return Coefficients 
 */
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

/**
 * @brief Peak filter
 * Peak filter for right channel
 * @param chainSettings 
 * @param sampleRate 
 * @return Coefficients 
 */
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

/**
 * @brief Update peak filter
 * Make left/right channel peak filter
 * @param chainSettings 
 */
void EQlibriumAudioProcessor::updatePeakFilter(const ChainSettings &chainSettings) {
    auto leftPeakCoefficients = makeLeftPeakFilter(chainSettings, getSampleRate());
    auto rightPeakCoefficients = makeRightPeakFilter(chainSettings, getSampleRate());
    updateCoefficients(leftChain.get<Peak>().coefficients, leftPeakCoefficients);
    updateCoefficients(rightChain.get<Peak>().coefficients, rightPeakCoefficients);
}

/**
 * @brief Update values
 * Replace old audio values with new values from filters 
 * @param old 
 * @param replacements 
 */
void updateCoefficients(Coefficients &old, const Coefficients &replacements) {
    *old = *replacements;
}

/**
 * @brief Update lowcut filter
 * Make left/right channel lowcut filter
 * @param chainSettings 
 */
void EQlibriumAudioProcessor::updateLowCutFilters(const ChainSettings& chainSettings) {
    auto leftLowCutCoefficients = makeLeftLowCutFilter(chainSettings, getSampleRate());
    auto rightLowCutCoefficients = makeRightLowCutFilter(chainSettings, getSampleRate());
    auto& leftLowCut = leftChain.get<LowCut>();
    auto& rightLowCut = rightChain.get<LowCut>();
    updateCutFilter(leftLowCut, leftLowCutCoefficients, chainSettings.leftLowCutSlope);
    updateCutFilter(rightLowCut, rightLowCutCoefficients, chainSettings.rightLowCutSlope);
}

/**
 * @brief Update highcut filter
 * Make left/right channel highcut filter
 * @param chainSettings 
 */
void EQlibriumAudioProcessor::updateHighCutFilters(const ChainSettings& chainSettings) {
    auto leftHighCutCoefficients = makeLeftHighCutFilter(chainSettings, getSampleRate());
    auto rightHighCutCoefficients = makeRightHighCutFilter(chainSettings, getSampleRate());
    auto& leftHighCut = leftChain.get<HighCut>();
    auto& rightHighCut = rightChain.get<HighCut>();
    updateCutFilter(leftHighCut, leftHighCutCoefficients, chainSettings.leftHighCutSlope);
    updateCutFilter(rightHighCut, rightHighCutCoefficients, chainSettings.rightHighCutSlope);
}

/**
 * @brief Update all filters
 * Update all filters
 */
void EQlibriumAudioProcessor::updateFilters() {
    auto chainSettings = getChainSettings(apvts);
    updatePeakFilter(chainSettings);
    updateLowCutFilters(chainSettings);
    updateHighCutFilters(chainSettings);
}

/**
 * @brief VU-level getter
 * Getter for VU-level meter values in ./level_meter/LevelMeter.cpp
 * @param channel 
 * @return float 
 */
float EQlibriumAudioProcessor::getRmsValue(const int channel) const {
    jassert(channel == 0 || channel == 1);
    if(channel == 0) {
        return rmsLevelLeft.getCurrentValue();
    } else if(channel == 1) {
        return rmsLevelRight.getCurrentValue();
    }
    return 0.f;
}

/**
 * @brief Layout
 * Set all sliders, buttons... (not visible)
 * @return juce::AudioProcessorValueTreeState::ParameterLayout 
 */
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
        str << " db/Okt";
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
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
        1.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Right Gain Slider",
        "Right Gain Slider",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
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
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "File Chooser Button",
        "File Chooser Button",
        true,
        attributes));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "Microphone Button",
        "Microphone Button",
        true,
        attributes));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "Save Button",
        "Save Button",
        true,
        attributes));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "Play Button",
        "Play Button",
        true,
        attributes));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "Replay Button",
        "Peplay Button",
        true,
        attributes));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "Microphone Button",
        "Microphone Button",
        true,
        attributes));
    return layout;
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new EQlibriumAudioProcessor(); }