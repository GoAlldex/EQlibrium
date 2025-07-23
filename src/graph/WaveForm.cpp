#include "WaveForm.hpp"

/**
 * @brief Construct a new Wave Form:: Wave Form object
 * Add timer and listener for repaint
 * Get waveform value from ./main/PluginProcessor.cpp
 * @param p 
 */
WaveForm::WaveForm(EQlibriumAudioProcessor& p) :
audioProcessor(p) {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->addListener(this);
    }
    thumbnail = p.getThumbnail();
    startTimerHz(30);
}

/**
 * @brief Destroy the Wave Form:: Wave Form object
 * Remove listener
 */
WaveForm::~WaveForm() {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->removeListener(this);
    }
}

/**
 * @brief Timer callback
 * Repaint waveform
 */
void WaveForm::timerCallback() {
    repaint();
}

/**
 * @brief Paint waveform
 * Paint left and right channel waveform
 * @param g 
 */
void WaveForm::paint(juce::Graphics& g) {
    using namespace juce;
    auto bounds = getBounds();
    bounds.setLeft(0);
    bounds.setTop(0);
    g.setColour(Colour(25,25,25));
    g.fillRect(bounds);
    bounds.removeFromBottom(3);
    bounds.removeFromTop(3);
    bounds.removeFromLeft(3);
    bounds.removeFromRight(3);
    g.setColour(Colour(51,51,255));
    thumbnail->drawChannels(g,
        bounds,
        0.0,
        thumbnail->getTotalLength(),
        1.f);
}

/**
 * @brief Resize
 * (Removable)
 */
void WaveForm::resized() {
    using namespace juce;
}

/**
 * @brief Parmater Changed
 * Set paramater changed for listener
 * @param parameterIndex 
 * @param newValue 
 */
void WaveForm::parameterValueChanged(int parameterIndex, float newValue) {
    parametersChanged.set(true);
}