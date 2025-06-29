#include "NormalImageButtons.hpp"

void LookAndFeelNormalImageButtons::drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) {
    using namespace juce;
    if(auto* pb = dynamic_cast<NormalImageButtons*>(&toggleButton) ) {
        auto bounds = toggleButton.getLocalBounds();
        g.setColour(Colour(25,25,25));
        g.fillRect(bounds);
        auto stateColor = toggleButton.getToggleState() ? Colour(0,255,128) : Colours::dimgrey;
        g.setColour(stateColor);
        g.drawRect(bounds, 1.f);
    }
}

void NormalImageButtons::paint(juce::Graphics &g) {
    using namespace juce;
    auto bounds = getLocalBounds();
    getLookAndFeel().drawToggleButton(
        g,
        *this,
        false,
        false
    );
}