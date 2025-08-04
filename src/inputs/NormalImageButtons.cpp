#include "NormalImageButtons.hpp"

/**
 * @brief LookAndFeel image buttons
 * Empty (no toggle state visual)
 * @param g 
 * @param toggleButton 
 * @param shouldDrawButtonAsHighlighted 
 * @param shouldDrawButtonAsDown 
 */
void LookAndFeelNormalImageButtons::drawToggleButton(
    juce::Graphics &g,
    juce::ToggleButton & toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown) {
    using namespace juce;
    if(auto* pb = dynamic_cast<NormalImageButtons*>(&toggleButton) ) {
        
    }
}

/**
 * @brief Paint
 * Draw image button
 * @param g 
 */
void NormalImageButtons::paint(juce::Graphics &g) {
    using namespace juce;
    auto bounds = getLocalBounds();
    getLookAndFeel().drawToggleButton(
        g,
        *this,
        false,
        false
    );
    auto img = EQImages(imageSet).getImage();
    g.drawImage(img, 0 ,0, bounds.getWidth(), bounds.getHeight(), 0, 0, img.getWidth(), img.getHeight(), false);
}