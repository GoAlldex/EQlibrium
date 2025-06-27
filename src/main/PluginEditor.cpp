
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
    gainSliderLeft(*audioProcessor.apvts.getParameter("Left Gain Slider"), "%"),
    gainSliderRight(*audioProcessor.apvts.getParameter("Right Gain Slider"), "%"),
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
    levelMeterRight(audioProcessor),
    leftGainAttachment(audioProcessor.apvts, "Left Gain Slider", gainSliderLeft),
    rightGainAttachment(audioProcessor.apvts, "Right Gain Slider", gainSliderRight),
    leftChannelButtonAttachment(audioProcessor.apvts, "Left Channel Button", channelButtonLeft),
    rightChannelButtonAttachment(audioProcessor.apvts, "Right Channel Button", channelButtonRight) {
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
    gainSliderLeft.labels.add({0.f, "L"});
    gainSliderRight.labels.add({0.f, "R"});
    channelButtonLeft.label = "L";
    channelButtonRight.label = "R";
    for(auto* comp : getComps()) {
        addAndMakeVisible(comp);
    }
    auto safePtr = SafePointer(this);
    channelButtonLeft.setLookAndFeel(&lnf);
    channelButtonLeft.onClick = [safePtr]() {
        if(auto* comp = safePtr.getComponent() ) {
            auto bypassed = comp->channelButtonLeft.getToggleState();
        }
    };
    channelButtonRight.setLookAndFeel(&lnf);
    channelButtonRight.onClick = [safePtr]() {
        if(auto* comp = safePtr.getComponent() ) {
            auto bypassed = comp->channelButtonRight.getToggleState();
        }
    };
    setSize (1000, 1000);
}

EQlibriumAudioProcessorEditor::~EQlibriumAudioProcessorEditor() {
    channelButtonLeft.setLookAndFeel(nullptr);
    channelButtonRight.setLookAndFeel(nullptr);
}

//==============================================================================
void EQlibriumAudioProcessorEditor::paint (juce::Graphics& g) {
    using namespace juce;
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
    g.setColour(Colour(25,25,25));
    g.fillRect(window_channel_label_left.toFloat());
    g.fillRect(window_channel_label_right.toFloat());
    auto path = File::getSpecialLocation (File::SpecialLocationType::currentExecutableFile).getSiblingFile("images").getFullPathName();
    Image left_channel_img = ImageFileFormat::loadFrom(File(path+"/left_channel.png"));
    g.drawImage(left_channel_img, 0, 400, 30, 400, 0, 0, 30,400, false);
    Image right_channel_img = ImageFileFormat::loadFrom(File(path+"/right_channel.png"));
    g.drawImage(right_channel_img, getLocalBounds().getWidth()-30, 400, 30, 400, 0, 0, 30,400, false);
}

void EQlibriumAudioProcessorEditor::resized() {
    window_full_rect = getLocalBounds();
    auto h = window_full_rect.getHeight();
    // Categorized places
    window_micro_rect = window_full_rect.removeFromTop(h/6);
    window_settings_rect = window_full_rect.removeFromTop(h/10);
    window_graph_rect = window_full_rect.removeFromTop(window_full_rect.getHeight());
    window_channel_label_left = window_graph_rect.removeFromLeft(30);
    window_channel_label_left.removeFromTop(3);
    window_channel_label_right = window_graph_rect.removeFromRight(30);
    window_channel_label_right.removeFromTop(3);
    window_inner_grap_rect = window_graph_rect.removeFromTop(window_graph_rect.getHeight());
    auto h_inner = window_inner_grap_rect.getHeight();
    window_filter_rect = window_inner_grap_rect.removeFromTop(h_inner/2-25);
    window_analyser_rect = window_inner_grap_rect.removeFromTop(h_inner/2-25);
    window_vumeter_rect = window_inner_grap_rect.removeFromTop(50);
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
    auto peakLD = juce::Rectangle(37, 270, peakL.getWidth()-2, peakL.getHeight()-2);
    auto peakWL = peakLD.getWidth();
    peakFreqSliderLeft.setBounds(peakLD.removeFromLeft(peakWL/3));
    peakGainSliderLeft.setBounds(peakLD.removeFromRight(peakWL/3));
    peakQualitySliderLeft.setBounds(peakLD.removeFromRight(peakWL/3));
    highCutL = window_filter_left_rect.removeFromTop(filterLeftH/3);
    highCutL.removeFromBottom(3);
    auto highCutLD = juce::Rectangle(37, 372, highCutL.getWidth()-2, highCutL.getHeight()-2);
    auto highCutWL = highCutLD.getWidth();
    highCutFreqSliderLeft.setBounds(highCutLD.removeFromLeft(highCutWL/2));
    highCutSlopeSliderLeft.setBounds(highCutLD.removeFromLeft(highCutWL/2));
    lowCutL = window_filter_left_rect.removeFromTop(filterLeftH/3);
    lowCutL.removeFromBottom(3);
    auto lowCutLD = juce::Rectangle(37, 474, lowCutL.getWidth()-2, lowCutL.getHeight()-2);
    auto lowCutWL = lowCutLD.getWidth();
    lowCutFreqSliderLeft.setBounds(lowCutLD.removeFromLeft(lowCutWL/2));
    lowCutSlopeSliderLeft.setBounds(lowCutLD.removeFromLeft(lowCutWL/2));
    filterLeft.setBounds(window_analyser_left_filter_rect);
    freqLeft.setBounds(window_analyser_left_freq_rect);
    //Set graph right
    auto filterRightH = window_filter_right_rect.getHeight();
    peakR = window_filter_right_rect.removeFromTop(filterRightH/3);
    peakR.removeFromBottom(3);
    auto peakRD = juce::Rectangle(peakR.getWidth()+44, 278, peakR.getWidth()-2, peakR.getHeight()-2);
    auto peakWR = peakRD.getWidth();
    peakFreqSliderRight.setBounds(peakRD.removeFromLeft(peakWR/3));
    peakGainSliderRight.setBounds(peakRD.removeFromRight(peakWR/3));
    peakQualitySliderRight.setBounds(peakRD.removeFromRight(peakWR/3));
    highCutR = window_filter_right_rect.removeFromTop(filterRightH/3);
    highCutR.removeFromBottom(3);
    auto highCutRD = juce::Rectangle(highCutR.getWidth()+44, 390, highCutR.getWidth()-2, highCutR.getHeight()-2);
    auto highCutWR = highCutRD.getWidth();
    highCutFreqSliderRight.setBounds(highCutRD.removeFromLeft(highCutWR/2));
    highCutSlopeSliderRight.setBounds(highCutRD.removeFromLeft(highCutWR/2));
    lowCutR = window_filter_right_rect.removeFromTop(filterRightH/3);
    lowCutR.removeFromBottom(3);
    auto lowCutRD = juce::Rectangle(lowCutR.getWidth()+44, 501, lowCutR.getWidth()-2, lowCutR.getHeight()-2);
    auto lowCutWR = lowCutRD.getWidth();
    lowCutFreqSliderRight.setBounds(lowCutRD.removeFromLeft(lowCutWR/2));
    lowCutSlopeSliderRight.setBounds(lowCutRD.removeFromLeft(lowCutWR/2));
    filterRight.setBounds(window_analyser_right_filter_rect);
    freqRight.setBounds(window_analyser_right_freq_rect);
    // Set Level-Meter
    levelMeterLeft.setBounds(window_vumeter_left_rect);
    levelMeterRight.setBounds(window_vumeter_right_rect);
    // Channel Volume
    auto gainBox = juce::Rectangle(12,195, 150, 50);
    auto gainL = gainBox.removeFromTop(gainBox.getHeight()/2);
    gainL.removeFromBottom(3);
    gainSliderLeft.setBounds(gainL);
    auto gainR = gainBox.removeFromTop(gainBox.getHeight());
    gainR.removeFromTop(3);
    gainSliderRight.setBounds(gainR);
    // Channel on/off Buttons
    auto onoffBox = juce::Rectangle(190,177, 37, 80);
    auto onoffL = onoffBox.removeFromTop(onoffBox.getHeight()/2);
    onoffL.removeFromBottom(3);
    channelButtonLeft.setBounds(onoffL);
    auto onoffR = onoffBox.removeFromTop(onoffBox.getHeight());
    onoffR.removeFromTop(3);
    channelButtonRight.setBounds(onoffR);
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
        &levelMeterRight,
        &gainSliderLeft,
        &gainSliderRight,
        &channelButtonLeft,
        &channelButtonRight
    };
}