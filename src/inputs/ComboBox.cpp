#include "ComboBox.hpp"

/** @brief ComboBox draw
 * 
 */

void LookAndFeelComboBox::drawComboBox(
juce::Graphics& g,
int width,
int height,
bool isButtonDown,
int buttonX,
int buttonY,
int buttonW,
int buttonH,
juce::ComboBox & comboBox) {
    using namespace juce;
    Colour borderColour = comboBox.findColour(juce::ComboBox::outlineColourId);
    Colour bgColour = comboBox.findColour(ComboBox::backgroundColourId);
    g.setColour(bgColour);
    g.fillRect(.0f, .0f, (float)width, (float)height);
    g.setColour(borderColour);
    g.drawRect(0.5f, 0.5f, (float)width-1.f, (float)height-1.f, 1.f);
    g.setColour(Colours::white);
    g.setFont(20);
    String text = comboBox.getText();
    g.drawText(text, comboBox.getLocalBounds().reduced(4, 0), Justification::centred, true);
}