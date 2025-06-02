#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"
#include "Utils.hpp"
#include "magic_enum/magic_enum.hpp"

/**
 * @brief Retrieves the current parameter values from the ValueTreeState
 * This method creates a new ChainSettings object and populates it with current parameter values
 */
AvSynthAudioProcessor::ChainSettings
AvSynthAudioProcessor::ChainSettings::Get(const juce::AudioProcessorValueTreeState &parameters) {
    ChainSettings settings{};

    // Load the gain parameter value using magic_enum to convert enum to string
    settings.gain = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::Gain>().data())->load();
    // Load the frequency parameter value using magic_enum to convert enum to string
    settings.frequency = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::Frequency>().data())->load();
    settings.oscType = static_cast<OscType>(
        static_cast<int>(parameters.getRawParameterValue(magic_enum::enum_name<Parameters::OscType>().data())->load()));
    settings.LowPassFreq = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::LowPassFreq>().data())->load();
    settings.HighPassFreq = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::HighPassFreq>().data())->load();

    return settings;
}

//==============================================================================
AvSynthAudioProcessor::AvSynthAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      ) {
}

AvSynthAudioProcessor::~AvSynthAudioProcessor() {}

//==============================================================================
const juce::String AvSynthAudioProcessor::getName() const { return JucePlugin_Name; }

bool AvSynthAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AvSynthAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AvSynthAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AvSynthAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int AvSynthAudioProcessor::getNumPrograms() {
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int AvSynthAudioProcessor::getCurrentProgram() { return 0; }

void AvSynthAudioProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }

const juce::String AvSynthAudioProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void AvSynthAudioProcessor::changeProgramName(int index, const juce::String &newName) { juce::ignoreUnused(index, newName); }

//==============================================================================
void AvSynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused(sampleRate);

    previousChainSettings = ChainSettings::Get(parameters);
    circularBuffer.setSize(1, samplesPerBlock);

    updateAngleDelta(previousChainSettings.frequency);

    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    updateLowPassCoefficients(previousChainSettings.LowPassFreq);
    updateHighPassCoefficients(previousChainSettings.HighPassFreq);
}

void AvSynthAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AvSynthAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

/**
 * @brief Main audio processing function called by the host to process a block of audio data
 * @param buffer Audio buffer containing the input/output audio data
 * @param midiMessages MIDI messages to be processed
 */
void AvSynthAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
    // Ignore MIDI messages as they're not used in this processor
    juce::ignoreUnused(midiMessages);
    // Prevent denormalized numbers in audio calculations for better performance
    juce::ScopedNoDenormals noDenormals;
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Merge keyboardComponent MIDI events into midiMessages
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    // If there is a MIDI message, process it and set the frequency to the value in the message
    if (!midiMessages.isEmpty()) {
        for (const auto &message : midiMessages) {
            if (message.getMessage().isNoteOn()) {
                float frequency =
                    static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(message.getMessage().getNoteNumber()));
                // Set the frequency based on the MIDI note number
                auto *freqParam = parameters.getParameter(magic_enum::enum_name<Parameters::Frequency>().data());
                if (auto *floatParam = dynamic_cast<juce::AudioParameterFloat *>(freqParam)) {
                    // The value needs to be normalized to the range of 0 to 1 for the parameter
                    float normValue = floatParam->convertTo0to1(frequency);
                    floatParam->setValueNotifyingHost(normValue);
                }
                break;
            }
        }
    }

    // Get current parameter values
    const auto chainSettings = ChainSettings::Get(parameters);

    // Check if the frequency has changed since the last process call
    if (!juce::approximatelyEqual(previousChainSettings.frequency, chainSettings.frequency)) {
        // Create a linear ramp to smoothly transition the frequency
        LinearRamp<float> frequencyRamp;
        frequencyRamp.reset(previousChainSettings.frequency, chainSettings.frequency, buffer.getNumSamples());

        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
            auto currentSample = getOscSample(chainSettings.oscType, currentAngle);
            currentAngle += angleDelta;
            updateAngleDelta(frequencyRamp.getNext());

            // Write the current sample to all output channels
            for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
                buffer.getWritePointer(channel)[sample] = currentSample;
            }
        }
    } else {
        // Sample the sine wave at the current sample rate and write the samples to the buffers
        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
            auto currentSample = getOscSample(chainSettings.oscType, currentAngle);
            currentAngle += angleDelta;
            // Write the current sample to all output channels
            for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
                buffer.getWritePointer(channel)[sample] = currentSample;
            }
        }
    }

    updateLowPassCoefficients(chainSettings.LowPassFreq);
    updateHighPassCoefficients(chainSettings.HighPassFreq);

    // Apply the filters to the audio buffer
    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);

    if (juce::approximatelyEqual(chainSettings.gain, previousChainSettings.gain)) {
        for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
            buffer.applyGain(channel, 0, buffer.getNumSamples(), previousChainSettings.gain);
        }
    } else {
        for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
            buffer.applyGainRamp(channel, 0, buffer.getNumSamples(), previousChainSettings.gain, chainSettings.gain);
        }
    }

    previousChainSettings = chainSettings;

    // Get pointer to the first channel's data (mono processing for simplicity)
    const float *channelData = buffer.getReadPointer(0);

    // Copy input samples to circular buffer for visualization
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        // Write current sample to circular buffer
        circularBuffer.setSample(0, bufferWritePos, channelData[sample]);
        // Increment write position and wrap around if necessary
        bufferWritePos = (bufferWritePos + 1) % circularBuffer.getNumSamples();
    }
}

//==============================================================================
bool AvSynthAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *AvSynthAudioProcessor::createEditor() {
    return new AvSynthAudioProcessorEditor(*this);
    // return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
// Save plugin state - called by the host when saving project or plugin preset
void AvSynthAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // This method stores parameters in the memory block.
    // ValueTree is used as an intermediate to save complex parameter data
    // Parameters are written to a memory stream which is then stored in the destData block

    // Create output stream and write parameter state to it
    juce::MemoryOutputStream stream(destData, true);
    parameters.state.writeToStream(stream);
}

void AvSynthAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        parameters.replaceState(tree);
    }
}
void AvSynthAudioProcessor::updateAngleDelta(float frequency) {
    auto cyclesPerSample = frequency / getSampleRate();
    angleDelta = cyclesPerSample * juce::MathConstants<double>::twoPi;
}

float AvSynthAudioProcessor::getOscSample(OscType type, double angle) {
    switch (type) {
    case OscType::Sine:
        return static_cast<float>(std::sin(angle));
    case OscType::Square:
        return (std::sin(angle) >= 0.0 ? 1.0f : -1.0f);
    case OscType::Saw:
        return static_cast<float>(
            2.0f * (angle / juce::MathConstants<double>::twoPi - std::floor(0.5 + angle / juce::MathConstants<double>::twoPi)));
    case OscType::Triangle:
        return 2.0f * static_cast<float>(std::abs(2.0f * (angle / juce::MathConstants<double>::twoPi -
                                                          std::floor(0.5 + angle / juce::MathConstants<double>::twoPi)))) -
               1.0f;
    default:
        return 0.0f;
    }
}
void AvSynthAudioProcessor::updateHighPassCoefficients(float frequency) {
    auto highPassCoefficients =
        juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(frequency, getSampleRate(), 4);

    auto &leftHighPass = leftChain.get<0>();
    *leftHighPass.get<0>().coefficients = *highPassCoefficients[0];
    *leftHighPass.get<1>().coefficients = *highPassCoefficients[1];

    auto &rightHighPass = rightChain.get<0>();
    *rightHighPass.get<0>().coefficients = *highPassCoefficients[0];
    *rightHighPass.get<1>().coefficients = *highPassCoefficients[1];
}

void AvSynthAudioProcessor::updateLowPassCoefficients(float frequency) {
    auto lowPassCoefficients =
        juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(frequency, getSampleRate(), 4);

    auto &leftLowPass = leftChain.get<1>();
    *leftLowPass.get<0>().coefficients = *lowPassCoefficients[0];
    *leftLowPass.get<1>().coefficients = *lowPassCoefficients[1];

    auto &rightLowPass = rightChain.get<1>();
    *rightLowPass.get<0>().coefficients = *lowPassCoefficients[0];
    *rightLowPass.get<1>().coefficients = *lowPassCoefficients[1];
}

template <typename ParamT, AvSynthAudioProcessor::Parameters Param, typename... Args>
static std::unique_ptr<ParamT> makeParameter(Args &&...args) {
    return std::make_unique<ParamT>(magic_enum::enum_name<Param>().data(), magic_enum::enum_name<Param>().data(),
                                    std::forward<Args>(args)...);
}

juce::AudioProcessorValueTreeState::ParameterLayout AvSynthAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::Gain>(0.0f, 1.0f, 0.25f));

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::Frequency>(
        juce::NormalisableRange(20.0f, 20000.0f, 1.0f, 0.3f), 440.0f));

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::HighPassFreq>(
        juce::NormalisableRange(20.0f, 20000.0f, 1.0f, 0.3f), 20.0f));

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::LowPassFreq>(
        juce::NormalisableRange(20.0f, 20000.0f, 1.0f, 0.3f), 20000.0f));

    layout.add(makeParameter<juce::AudioParameterChoice, Parameters::OscType>(
        juce::StringArray{magic_enum::enum_name<OscType::Sine>().data(), magic_enum::enum_name<OscType::Saw>().data(),
                          magic_enum::enum_name<OscType::Square>().data(), magic_enum::enum_name<OscType::Triangle>().data()},
        0));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new AvSynthAudioProcessor(); }
