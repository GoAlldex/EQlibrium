#include "ChannelButtons.hpp"

/**
 * @brief LookAndFeel channel Button
 * Draw border green or dimgrey (Toggle state true/false)
 * @param g 
 * @param toggleButton 
 * @param shouldDrawButtonAsHighlighted 
 * @param shouldDrawButtonAsDown 
 */
void LookAndFeelChannelButtons::drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) {
    using namespace juce;
    if(auto* pb = dynamic_cast<ChannelButtons*>(&toggleButton) ) {
        auto bounds = toggleButton.getLocalBounds();
        g.setColour(Colour(25,25,25));
        g.fillRect(bounds);
        auto stateColor = toggleButton.getToggleState() ? Colour(0,255,128) : Colours::dimgrey;
        g.setColour(stateColor);
        g.drawRect(bounds, 1.f);
    }
}

/**
 * @brief Paint
 * Draw button with char R or L
 * @param g 
 */
void ChannelButtons::paint(juce::Graphics &g) {
    using namespace juce;
    auto bounds = getLocalBounds();
    getLookAndFeel().drawToggleButton(
        g,
        *this,
        false,
        false
    );
    g.setColour(Colours::white);
    g.setFont(20);
    g.drawFittedText(label, bounds.toNearestInt(), juce::Justification::centred, 1);
}