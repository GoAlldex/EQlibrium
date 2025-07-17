#include "GlowImageButtons.hpp"

/**
 * @brief LookAndFeel glow image button
 * Draw glow image
 * Toggle state false = white replay image
 * Toggle state true = green replay image
 * @param g 
 * @param toggleButton 
 * @param shouldDrawButtonAsHighlighted 
 * @param shouldDrawButtonAsDown 
 */
void LookAndFeelGlowButton::drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) {
    using namespace juce;
    if(auto* pb = dynamic_cast<GlowImageButtons*>(&toggleButton) ) {
        auto bounds = pb->getLocalBounds();
        Colour imgColour = toggleButton.getToggleState() ? Colour(0,255,128) : Colour(255,255,255);
        Image img = EQImages(pb->image).getImage();
        g.setColour(imgColour);
        g.drawImage(img, 1, 1, bounds.getWidth()-2, bounds.getHeight()-2, 0, 0, img.getWidth(), img.getHeight(), true);
    }
}

/**
 * @brief Paint
 * Draw glow image button
 * @param g 
 */
void GlowImageButtons::paint(juce::Graphics &g) {
    using namespace juce;
    getLookAndFeel().drawToggleButton(
        g,
        *this,
        false,
        false
    );
}