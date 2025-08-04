#include "PlayButton.hpp"

/**
 * @brief LookAndFeel play button
 * Draw play image
 * Draw pause image
 * Toggle state false = play image
 * Toggle state true = pause image
 * @param g 
 * @param toggleButton 
 * @param shouldDrawButtonAsHighlighted 
 * @param shouldDrawButtonAsDown 
 */
void LookAndFeelPlayButton::drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) {
    using namespace juce;
    if(auto* pb = dynamic_cast<PlayButton*>(&toggleButton) ) {
        auto bounds = pb->getLocalBounds();
        Image img = toggleButton.getToggleState() ? EQImages(pb->imagePause).getImage() : EQImages(pb->imagePlay).getImage();
        g.drawImage(img, 5, 5, bounds.getWidth()-10, bounds.getHeight()-10, 0, 0, img.getWidth(), img.getHeight(), false);
    }
}

/**
 * @brief Paint
 * Draw play/pause button
 * @param g 
 */
void PlayButton::paint(juce::Graphics &g) {
    using namespace juce;
    auto smRect = Rectangle(1, 1, getLocalBounds().getWidth()-2, getLocalBounds().getHeight()-2);
    g.setColour(Colours::white);
    g.drawEllipse(smRect.toFloat(), 1.5f);
    getLookAndFeel().drawToggleButton(
        g,
        *this,
        false,
        false
    );
}