#include "./ToolTip.hpp"

/** @brief Tooltip
 * Look and feel Tooltip
 */
void LookAndFeelToolTip::drawTooltip(juce::Graphics& g, const juce::String& text,int width, int height) {
    using namespace juce;
    auto bounds = juce::Rectangle<int>(width, height);
    g.fillAll(Colour(50,50,50));
    g.setColour(Colour(0,255,128));
    g.drawRect(bounds, 1.f);
    auto textBounds = juce::Rectangle<int>(width, height).reduced(3, 3);
    g.setColour(Colours::white);
    g.setFont(14);
    g.drawFittedText(text, textBounds, Justification::topLeft, 3, 1.f);
}