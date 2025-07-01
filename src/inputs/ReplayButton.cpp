#include "ReplayButton.hpp"

void LookAndFeelReplayButton::drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) {
    using namespace juce;
    if(auto* pb = dynamic_cast<ReplayButton*>(&toggleButton) ) {
        auto bounds = pb->getLocalBounds();
        Colour imgColour = toggleButton.getToggleState() ? Colour(0,255,128) : Colour(255,255,255);
        Image img = EQImages(pb->image).getImage();
        g.setColour(imgColour);
        g.drawImage(img, 1, 1, bounds.getWidth()-2, bounds.getHeight()-2, 0, 0, img.getWidth(), img.getHeight(), true);
    }
}

void ReplayButton::paint(juce::Graphics &g) {
    using namespace juce;
    getLookAndFeel().drawToggleButton(
        g,
        *this,
        false,
        false
    );
}