#include "TextButtons.hpp"

/** @brief ComboBox
 * Not used ComboBox == ignore
 */

void LookAndFeelTextButtons::drawComboBox(
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
    if(auto* pb = dynamic_cast<TextButtons*>(&comboBox) ) {
        auto bounds = comboBox.getLocalBounds();
        g.setColour(Colour(25,25,25));
        g.fillRect(bounds);
        /*if(pb->getPos() == (pb->label.size()-1)) {
            pb->setPos(0);
        } else {
            pb->setPos(pb->getPos()+1);
        }
        g.setColour(Colours::white);
        g.setFont(16);
        g.drawFittedText(pb->label[pb->getPos()], bounds.toNearestInt(), juce::Justification::centred, pb->label[pb->getPos()].length());*/
    }
}

void TextButtons::paint(juce::Graphics &g) {
    using namespace juce;
    auto bounds = getLocalBounds();
    getLookAndFeel().drawComboBox(
        g,
        150,
        50,
        true,
        150,
        50,
        150,
        50,
        *this
    );
    /*g.setColour(Colours::white);
    g.setFont(16);
    g.drawFittedText(label[pos], bounds.toNearestInt(), juce::Justification::centred, 1);*/
}