
#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

//==============================================================================
EQlibriumAudioProcessorEditor::EQlibriumAudioProcessorEditor (EQlibriumAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    peakFreqSliderLeft(*audioProcessor.apvts.getParameter("Left Peak Freq"), "Hz"),
    peakGainSliderLeft(*audioProcessor.apvts.getParameter("Left Peak Gain"), "dB"),
    peakQualitySliderLeft(*audioProcessor.apvts.getParameter("Left Peak Quality"), ""),
    lowCutFreqSliderLeft(*audioProcessor.apvts.getParameter("Left LowCut Freq"), "Hz"),
    lowCutSlopeSliderLeft(*audioProcessor.apvts.getParameter("Left LowCut Slope"), "dB/Okt"),
    highCutFreqSliderLeft(*audioProcessor.apvts.getParameter("Left HighCut Freq"), "Hz"),
    highCutSlopeSliderLeft(*audioProcessor.apvts.getParameter("Left HighCut Slope"), "dB/Okt"),
    peakFreqSliderRight(*audioProcessor.apvts.getParameter("Right Peak Freq"), "Hz"),
    peakGainSliderRight(*audioProcessor.apvts.getParameter("Right Peak Gain"), "dB"),
    peakQualitySliderRight(*audioProcessor.apvts.getParameter("Right Peak Quality"), ""),
    lowCutFreqSliderRight(*audioProcessor.apvts.getParameter("Right LowCut Freq"), "Hz"),
    lowCutSlopeSliderRight(*audioProcessor.apvts.getParameter("Right LowCut Slope"), "dB/Okt"),
    highCutFreqSliderRight(*audioProcessor.apvts.getParameter("Right HighCut Freq"), "Hz"),
    highCutSlopeSliderRight(*audioProcessor.apvts.getParameter("Right HighCut Slope"), "dB/Okt"),
    filterLeft(audioProcessor),
    freqLeft(audioProcessor),
    leftPeakFreqSliderAttachment(audioProcessor.apvts, "Left Peak Freq", peakFreqSliderLeft),
    leftPeakGainSliderAttachment(audioProcessor.apvts, "Left Peak Gain", peakGainSliderLeft),
    leftPeakQualitySliderAttachment(audioProcessor.apvts, "Left Peak Quality", peakQualitySliderLeft),
    leftLowCutFreqSliderAttachment(audioProcessor.apvts, "Left LowCut Freq", lowCutFreqSliderLeft),
    leftLowCutSlopeSliderAttachment(audioProcessor.apvts, "Left LowCut Slope", lowCutSlopeSliderLeft),
    leftHighCutFreqSliderAttachment(audioProcessor.apvts, "Left HighCut Freq", highCutFreqSliderLeft),
    leftHighCutSlopeSliderAttachment(audioProcessor.apvts, "Left HighCut Slope", highCutSlopeSliderLeft),
    rightPeakFreqSliderAttachment(audioProcessor.apvts, "Right Peak Freq", peakFreqSliderRight),
    rightPeakGainSliderAttachment(audioProcessor.apvts, "Right Peak Gain", peakGainSliderRight),
    rightPeakQualitySliderAttachment(audioProcessor.apvts, "Right Peak Quality", peakQualitySliderRight),
    rightLowCutFreqSliderAttachment(audioProcessor.apvts, "Right LowCut Freq", lowCutFreqSliderRight),
    rightLowCutSlopeSliderAttachment(audioProcessor.apvts, "Right LowCut Slope", lowCutSlopeSliderRight),
    rightHighCutFreqSliderAttachment(audioProcessor.apvts, "Right HighCut Freq", highCutFreqSliderRight),
    rightHighCutSlopeSliderAttachment(audioProcessor.apvts, "Right HighCut Slope", highCutSlopeSliderRight),
    filterRight(audioProcessor),
    freqRight(audioProcessor),
    levelMeterLeft(audioProcessor),
    levelMeterRight(audioProcessor) {
    peakFreqSliderLeft.labels.add({0.f, "20Hz"});
    peakFreqSliderLeft.labels.add({1.f, "20kHz"});
    peakGainSliderLeft.labels.add({0.f, "-24dB"});
    peakGainSliderLeft.labels.add({1.f, "+24dB"});
    peakQualitySliderLeft.labels.add({0.f, "0.1"});
    peakQualitySliderLeft.labels.add({1.f, "10"});
    lowCutFreqSliderLeft.labels.add({0.f, "20Hz"});
    lowCutFreqSliderLeft.labels.add({1.f, "20kHz"});
    highCutFreqSliderLeft.labels.add({0.f, "20Hz"});
    highCutFreqSliderLeft.labels.add({1.f, "20kHz"});
    lowCutSlopeSliderLeft.labels.add({0.f, "12"});
    lowCutSlopeSliderLeft.labels.add({1.f, "48"});
    highCutSlopeSliderLeft.labels.add({0.f, "12"});
    highCutSlopeSliderLeft.labels.add({1.f, "48"});
    peakFreqSliderRight.labels.add({0.f, "20Hz"});
    peakFreqSliderRight.labels.add({1.f, "20kHz"});
    peakGainSliderRight.labels.add({0.f, "-24dB"});
    peakGainSliderRight.labels.add({1.f, "+24dB"});
    peakQualitySliderRight.labels.add({0.f, "0.1"});
    peakQualitySliderRight.labels.add({1.f, "10"});
    lowCutFreqSliderRight.labels.add({0.f, "20Hz"});
    lowCutFreqSliderRight.labels.add({1.f, "20kHz"});
    highCutFreqSliderRight.labels.add({0.f, "20Hz"});
    highCutFreqSliderRight.labels.add({1.f, "20kHz"});
    lowCutSlopeSliderRight.labels.add({0.f, "12"});
    lowCutSlopeSliderRight.labels.add({1.f, "48"});
    highCutSlopeSliderRight.labels.add({0.f, "12"});
    highCutSlopeSliderRight.labels.add({1.f, "48"});
    for(auto* comp : getComps()) {
        addAndMakeVisible(comp);
    }
    setSize (1000, 1000);
}
EQlibriumAudioProcessorEditor::~EQlibriumAudioProcessorEditor() { }

//==============================================================================
void EQlibriumAudioProcessorEditor::paint (juce::Graphics& g) {
    using namespace juce;
    /*
    // DEBUG LAYOUT
    // Show layout
    g.setColour(Colours::red);
    g.drawRect(window_micro_rect.toFloat(), 0.1f);
    g.setColour(Colours::red);
    g.drawRect(window_settings_rect.toFloat(), 0.1f);
    g.setColour(Colours::red);
    g.drawRect(window_filter_rect.toFloat(), 0.1f);
    g.setColour(Colours::red);
    g.drawRect(window_analyser_rect.toFloat(), 0.1f);
    g.setColour(Colours::red);
    g.drawRect(window_vumeter_rect.toFloat(), 0.1f);
    // Show channel split
    g.setColour(Colours::green);
    g.drawRect(window_filter_left_rect.toFloat(), 0.1f);
    g.drawRect(window_analyser_left_rect.toFloat(), 0.1f);
    g.drawRect(window_vumeter_left_rect.toFloat(), 0.1f);
    g.setColour(Colours::blue);
    g.drawRect(window_filter_right_rect.toFloat(), 0.1f);
    g.drawRect(window_analyser_right_rect.toFloat(), 0.1f);
    g.drawRect(window_vumeter_right_rect.toFloat(), 0.1f);
    // Show graph split
    g.setColour(Colours::white);
    g.drawRect(window_analyser_left_filter_rect.toFloat(), 0.1f);
    g.drawRect(window_analyser_right_filter_rect.toFloat(), 0.1f);
    g.setColour(Colours::white);
    g.drawRect(window_analyser_left_freq_rect.toFloat(), 0.1f);
    g.drawRect(window_analyser_right_freq_rect.toFloat(), 0.1f);*/
    Colour bgColour = Colour(50,50,50);
    Colour lineColour = Colour(25,25,25);
    g.setColour(bgColour);
    g.fillRect(getLocalBounds());
    g.setColour(lineColour);
    g.drawRect(window_micro_rect.toFloat(), 0.3f);
    g.drawRect(window_settings_rect.toFloat(), 0.3f);
    g.drawRect(peakL.toFloat(), 0.3f);
    g.drawRect(peakR.toFloat(), 0.3f);
    g.drawRect(highCutL.toFloat(), 0.3f);
    g.drawRect(highCutR.toFloat(), 0.3f);
    g.drawRect(lowCutL.toFloat(), 0.3f);
    g.drawRect(lowCutR.toFloat(), 0.3f);
    g.fillRect(window_analyser_left_filter_rect);
    g.fillRect(window_analyser_right_filter_rect);
    g.fillRect(window_analyser_left_freq_rect);
    g.fillRect(window_analyser_right_freq_rect);
}

void EQlibriumAudioProcessorEditor::resized() {
    window_full_rect = getLocalBounds();
    auto h = window_full_rect.getHeight();
    // Categorized places
    window_micro_rect = window_full_rect.removeFromTop(h/6);
    window_settings_rect = window_full_rect.removeFromTop(h/10);
    window_filter_rect = window_full_rect.removeFromTop(h*19/30/2);
    window_analyser_rect = window_full_rect.removeFromTop(h*19/30/2);
    window_vumeter_rect = window_full_rect.removeFromTop(h/10);
    // Padding and margin micro rect
    window_micro_rect.removeFromTop(6);
    window_micro_rect.removeFromBottom(3);
    window_micro_rect.removeFromLeft(6);
    window_micro_rect.removeFromRight(6);
    // Padding and margin settings rect
    window_settings_rect.removeFromTop(3);
    window_settings_rect.removeFromBottom(3);
    window_settings_rect.removeFromLeft(6);
    window_settings_rect.removeFromRight(6);
    // L/R channel splitted filter rects
    window_filter_left_rect = window_filter_rect.removeFromLeft(window_filter_rect.getWidth()/2);
    window_filter_right_rect = window_filter_rect.removeFromLeft(window_filter_rect.getWidth());
    // Padding and margin filter rects
    window_filter_left_rect.removeFromTop(3);
    window_filter_left_rect.removeFromBottom(3);
    window_filter_left_rect.removeFromLeft(6);
    window_filter_left_rect.removeFromRight(3);
    window_filter_right_rect.removeFromTop(3);
    window_filter_right_rect.removeFromBottom(3);
    window_filter_right_rect.removeFromLeft(3);
    window_filter_right_rect.removeFromRight(6);
    // L/R channel splitted analyser rects
    window_analyser_left_rect = window_analyser_rect.removeFromLeft(window_analyser_rect.getWidth()/2);
    window_analyser_right_rect = window_analyser_rect.removeFromLeft(window_analyser_rect.getWidth());
    // Padding and margin analyser rects
    window_analyser_left_rect.removeFromTop(3);
    window_analyser_left_rect.removeFromBottom(3);
    window_analyser_left_rect.removeFromLeft(6);
    window_analyser_left_rect.removeFromRight(3);
    window_analyser_right_rect.removeFromTop(3);
    window_analyser_right_rect.removeFromBottom(3);
    window_analyser_right_rect.removeFromLeft(3);
    window_analyser_right_rect.removeFromRight(6);
    // Analyzer split filter and frequency graph
    window_analyser_left_filter_rect = window_analyser_left_rect.removeFromTop(window_analyser_left_rect.getHeight()/2);
    window_analyser_left_filter_rect.removeFromBottom(3);
    window_analyser_left_freq_rect = window_analyser_left_rect.removeFromTop(window_analyser_left_rect.getHeight());
    window_analyser_left_freq_rect.removeFromTop(3);
    window_analyser_right_filter_rect = window_analyser_right_rect.removeFromTop(window_analyser_right_rect.getHeight()/2);
    window_analyser_right_filter_rect.removeFromBottom(3);
    window_analyser_right_freq_rect = window_analyser_right_rect.removeFromTop(window_analyser_right_rect.getHeight());
    window_analyser_right_freq_rect.removeFromTop(3);
    // L/R channel splitted uv-meter rects
    window_vumeter_left_rect = window_vumeter_rect.removeFromLeft(window_vumeter_rect.getWidth()/2);
    window_vumeter_right_rect = window_vumeter_rect.removeFromLeft(window_vumeter_rect.getWidth());
    // Padding and margin uv-meter rects
    window_vumeter_left_rect.removeFromTop(3);
    window_vumeter_left_rect.removeFromBottom(6);
    window_vumeter_left_rect.removeFromLeft(6);
    window_vumeter_left_rect.removeFromRight(3);
    window_vumeter_right_rect.removeFromTop(3);
    window_vumeter_right_rect.removeFromBottom(6);
    window_vumeter_right_rect.removeFromLeft(3);
    window_vumeter_right_rect.removeFromRight(6);
    // Set graph left
    auto filterLeftH = window_filter_left_rect.getHeight();
    peakL = window_filter_left_rect.removeFromTop(filterLeftH/3);
    peakL.removeFromBottom(3);
    auto peakLD = juce::Rectangle(7, 270, peakL.getWidth()-2, peakL.getHeight()-2);
    auto peakWL = peakLD.getWidth();
    peakFreqSliderLeft.setBounds(peakLD.removeFromLeft(peakWL/3));
    peakGainSliderLeft.setBounds(peakLD.removeFromRight(peakWL/3));
    peakQualitySliderLeft.setBounds(peakLD.removeFromRight(peakWL/3));
    highCutL = window_filter_left_rect.removeFromTop(filterLeftH/3);
    highCutL.removeFromBottom(3);
    auto highCutLD = juce::Rectangle(7, 372, highCutL.getWidth()-2, highCutL.getHeight()-2);
    auto highCutWL = highCutLD.getWidth();
    highCutFreqSliderLeft.setBounds(highCutLD.removeFromLeft(highCutWL/2));
    highCutSlopeSliderLeft.setBounds(highCutLD.removeFromLeft(highCutWL/2));
    lowCutL = window_filter_left_rect.removeFromTop(filterLeftH/3);
    lowCutL.removeFromBottom(3);
    auto lowCutLD = juce::Rectangle(7, 474, lowCutL.getWidth()-2, lowCutL.getHeight()-2);
    auto lowCutWL = lowCutLD.getWidth();
    lowCutFreqSliderLeft.setBounds(lowCutLD.removeFromLeft(lowCutWL/2));
    lowCutSlopeSliderLeft.setBounds(lowCutLD.removeFromLeft(lowCutWL/2));
    filterLeft.setBounds(window_analyser_left_filter_rect);
    freqLeft.setBounds(window_analyser_left_freq_rect);
    //Set graph right
    auto filterRightH = window_filter_right_rect.getHeight();
    peakR = window_filter_right_rect.removeFromTop(filterRightH/3);
    peakR.removeFromBottom(3);
    auto peakRD = juce::Rectangle(peakR.getWidth()+14, 270, peakR.getWidth()-2, peakR.getHeight()-2);
    auto peakWR = peakRD.getWidth();
    peakFreqSliderRight.setBounds(peakRD.removeFromLeft(peakWR/3));
    peakGainSliderRight.setBounds(peakRD.removeFromRight(peakWR/3));
    peakQualitySliderRight.setBounds(peakRD.removeFromRight(peakWR/3));
    highCutR = window_filter_right_rect.removeFromTop(filterRightH/3);
    highCutR.removeFromBottom(3);
    auto highCutRD = juce::Rectangle(highCutR.getWidth()+14, 372, highCutR.getWidth()-2, highCutR.getHeight()-2);
    auto highCutWR = highCutRD.getWidth();
    highCutFreqSliderRight.setBounds(highCutRD.removeFromLeft(highCutWR/2));
    highCutSlopeSliderRight.setBounds(highCutRD.removeFromLeft(highCutWR/2));
    lowCutR = window_filter_right_rect.removeFromTop(filterRightH/3);
    lowCutR.removeFromBottom(3);
    auto lowCutRD = juce::Rectangle(lowCutR.getWidth()+14, 474, lowCutR.getWidth()-2, lowCutR.getHeight()-2);
    auto lowCutWR = lowCutRD.getWidth();
    lowCutFreqSliderRight.setBounds(lowCutRD.removeFromLeft(lowCutWR/2));
    lowCutSlopeSliderRight.setBounds(lowCutRD.removeFromLeft(lowCutWR/2));
    filterRight.setBounds(window_analyser_right_filter_rect);
    freqRight.setBounds(window_analyser_right_freq_rect);
    // Set Level-Meter
    levelMeterLeft.setBounds(window_vumeter_left_rect);
    levelMeterRight.setBounds(window_vumeter_right_rect);
}

std::vector<juce::Component*> EQlibriumAudioProcessorEditor::getComps()
{
    return {
        &peakFreqSliderLeft,
        &peakGainSliderLeft,
        &peakQualitySliderLeft,
        &lowCutFreqSliderLeft,
        &highCutFreqSliderLeft,
        &lowCutSlopeSliderLeft,
        &highCutSlopeSliderLeft,
        &filterLeft,
        &freqLeft,
        &peakFreqSliderRight,
        &peakGainSliderRight,
        &peakQualitySliderRight,
        &lowCutFreqSliderRight,
        &highCutFreqSliderRight,
        &lowCutSlopeSliderRight,
        &highCutSlopeSliderRight,
        &filterRight,
        &freqRight,
        &levelMeterLeft,
        &levelMeterRight
    };
}