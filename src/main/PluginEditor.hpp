#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.hpp"
#include "../graph/ResponseCurveComponent.hpp"
#include "../graph/PathProducer.hpp"
#include "../inputs/RotarySlider.hpp"
#include "../level_meter/LevelMeter.hpp"
#include "../inputs/LoudnessSlider.hpp"
#include "../inputs/ChannelButtons.hpp"
#include "../inputs/NormalImageButtons.hpp"
#include "../image/EQImages.hpp"
#include "../graph/WaveForm.hpp"
#include "../inputs/PlayButton.hpp"
#include "../inputs/GlowImageButtons.hpp"
#include "../inputs/TextButtons.hpp"

//==============================================================================

/**
 * @brief Include header + definition
 * All visuals variables in this standalone/VTS3 are definded down here
 */
class EQlibriumAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    EQlibriumAudioProcessorEditor (EQlibriumAudioProcessor&);
    ~EQlibriumAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    EQlibriumAudioProcessor& audioProcessor;
    juce::Rectangle<int> window_full_rect,
    window_micro_rect,
    window_settings_rect,
    window_graph_rect,
    window_channel_label_left,
    window_channel_label_right,
    window_inner_grap_rect,
    window_filter_rect,
    window_filter_left_rect,
    window_filter_right_rect,
    window_analyser_rect,
    window_analyser_left_rect,
    window_analyser_left_filter_rect,
    window_analyser_left_freq_rect,
    window_analyser_right_rect,
    window_analyser_right_filter_rect,
    window_analyser_right_freq_rect,
    window_vumeter_rect,
    window_vumeter_left_rect,
    window_vumeter_right_rect,
    peakLeftLabel,
    peakL,
    peakRightLabel,
    peakR,
    highCutLeftLabel,
    highCutL,
    highCutRightLabel,
    highCutR,
    lowCutLeftLabel,
    lowCutL,
    lowCutRightLabel,
    lowCutR;
    RotarySliderWithLabels peakFreqSliderLeft,
        peakGainSliderLeft,
        peakQualitySliderLeft,
        lowCutFreqSliderLeft,
        highCutFreqSliderLeft,
        lowCutSlopeSliderLeft,
        highCutSlopeSliderLeft,
        peakFreqSliderRight,
        peakGainSliderRight,
        peakQualitySliderRight,
        lowCutFreqSliderRight,
        highCutFreqSliderRight,
        lowCutSlopeSliderRight,
        highCutSlopeSliderRight;
    LinearSliderWithLabels gainSliderLeft, gainSliderRight;
    ResponseCurveComponent filterLeft, filterRight;
    PathProducerComponent freqLeft, freqRight;
    ChannelButtons channelButtonLeft, channelButtonRight;
    NormalImageButtons fileChooserButton, saveButton;
    WaveForm waveGraph;
    PlayButton playButton;
    GlowImageButtons replayButton, microphoneButton;
    juce::ComboBox equalizerInputButton;
    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;
    SliderAttachment leftPeakFreqSliderAttachment,
        leftPeakGainSliderAttachment,
        leftPeakQualitySliderAttachment,
        leftLowCutFreqSliderAttachment,
        leftHighCutFreqSliderAttachment,
        leftLowCutSlopeSliderAttachment,
        leftHighCutSlopeSliderAttachment,
        rightPeakFreqSliderAttachment,
        rightPeakGainSliderAttachment,
        rightPeakQualitySliderAttachment,
        rightLowCutFreqSliderAttachment,
        rightHighCutFreqSliderAttachment,
        rightLowCutSlopeSliderAttachment,
        rightHighCutSlopeSliderAttachment,
        leftGainAttachment,
        rightGainAttachment;
    using ButtonAttachment = APVTS::ButtonAttachment;
    ButtonAttachment leftChannelButtonAttachment,
        rightChannelButtonAttachment,
        fileChooserAttachment,
        microphoneAttachment,
        saveAttachment,
        playAttachment,
        replayAttachment;
    using ComboBoxAttachment = APVTS::ComboBoxAttachment;
    ComboBoxAttachment equalizerInputAttachment;
    LevelMeterComponent levelMeterLeft, levelMeterRight;
    std::vector<Component*> getComps();
    LookAndFeelChannelButtons lnfCh;
    LookAndFeelNormalImageButtons lnfImgBtn;
    LookAndFeelPlayButton lnfPlay;
    LookAndFeelGlowButton lnfGlow;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQlibriumAudioProcessorEditor)
};