#pragma once

#include <JuceHeader.h>
#include "../main/PluginProcessor.hpp"

/** @brief Tooltip
 * Define look and feel Tooltip
 */
struct LookAndFeelToolTip : juce::LookAndFeel_V4 {
    void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override;
};