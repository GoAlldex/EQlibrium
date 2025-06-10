#include "PluginEditor.hpp"
#include "PluginProcessor.hpp"

#include <magic_enum/magic_enum.hpp>

//==============================================================================
AvSynthAudioProcessorEditor::AvSynthAudioProcessorEditor(AvSynthAudioProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p),
      gainSlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      gainAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Gain>().data(), gainSlider),

      frequencySlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      frequencyAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Frequency>().data(),
                          frequencySlider),

      oscTypeComboBox(),
      oscTypeAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::OscType>().data(),
                        oscTypeComboBox),

      lowCutFreqSlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      lowCutFreqAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::LowPassFreq>().data(),
                           lowCutFreqSlider),

      highCutFreqSlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      highCutFreqAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::HighPassFreq>().data(),
                            highCutFreqSlider),

      keyboardComponent(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),

      waveformComponent(p.circularBuffer, p.bufferWritePos) {
    juce::ignoreUnused(processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    auto *oscTypeParam = dynamic_cast<juce::AudioParameterChoice *>(
        p.parameters.getParameter(magic_enum::enum_name<AvSynthAudioProcessor::Parameters::OscType>().data()));

    if (oscTypeParam != nullptr) {
        oscTypeComboBox.clear();
        auto &choices = oscTypeParam->choices;
        for (int i = 0; i < choices.size(); ++i) {
            oscTypeComboBox.addItem(choices[i], i + 1);
        }
        oscTypeComboBox.setSelectedId(oscTypeParam->getIndex() + 1, juce::dontSendNotification);
    }

    for (const auto component : GetComps()) {
        addAndMakeVisible(component);
    }
    setSize(400, 300);
    setResizable(true, true);
}

AvSynthAudioProcessorEditor::~AvSynthAudioProcessorEditor() {}

//==============================================================================
void AvSynthAudioProcessorEditor::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AvSynthAudioProcessorEditor::resized() {
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds().reduced(10);
    auto gainSliderArea = bounds.removeFromTop(40);
    auto frequencySliderArea = bounds.removeFromTop(40);
    auto oscTypeComboBoxArea = bounds.removeFromTop(40);
    auto lowCutFreqArea = bounds.removeFromTop(40);
    auto highCutFreqArea = bounds.removeFromTop(40);
    auto keyboardArea = bounds.removeFromTop(80);
    auto waveformArea = bounds.removeFromTop(bounds.getHeight() - 40);

    gainSlider.setBounds(gainSliderArea);
    frequencySlider.setBounds(frequencySliderArea);
    oscTypeComboBox.setBounds(oscTypeComboBoxArea);
    lowCutFreqSlider.setBounds(lowCutFreqArea);
    highCutFreqSlider.setBounds(highCutFreqArea);
    keyboardComponent.setBounds(keyboardArea);
    waveformComponent.setBounds(waveformArea);
}

std::vector<juce::Component *> AvSynthAudioProcessorEditor::GetComps() {
    return {&waveformComponent, &gainSlider,        &frequencySlider,  &oscTypeComboBox,
            &lowCutFreqSlider,  &highCutFreqSlider, &keyboardComponent};
}
