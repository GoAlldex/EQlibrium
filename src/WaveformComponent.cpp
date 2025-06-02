#include "WaveformComponent.hpp"

/**
 * @brief A component that displays an audio waveform visualization
 * This component continuously updates to show the current state of an audio buffer
 */

/**
 * @brief Constructs the WaveformComponent
 * @param bufferRef Reference to the audio buffer to visualize
 * @param writePosRef Reference to the current write position in the buffer
 */
WaveformComponent::WaveformComponent(juce::AudioSampleBuffer &bufferRef, int &writePosRef)
    : buffer(bufferRef), writePos(writePosRef) {
    startTimerHz(60); // Starts timer to refresh display at ~60 frames per second
}

/**
 * @brief Handles the painting of the component
 * @param g The graphics context used for drawing
 */
void WaveformComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::black);  // Set background to black
    g.setColour(juce::Colours::lime); // Set waveform color to lime
    drawWaveform(g);                  // Draw the actual waveform
}

/**
 * @brief Timer callback that triggers repainting
 * Called approximately 60 times per second to update the display
 */
void WaveformComponent::timerCallback() { repaint(); }

/**
 * @brief Draws the waveform visualization
 * @param g The graphics context used for drawing
 */
void WaveformComponent::drawWaveform(juce::Graphics &g) const {
    auto width = getWidth();
    auto height = getHeight();

    juce::Path waveformPath;
    waveformPath.startNewSubPath(0.f, height / 2.f); // Start path at vertical center

    const int numSamples = buffer.getNumSamples();
    const float step = static_cast<float>(numSamples) / width; // Calculate samples per pixel
    const int start = (writePos + 1) % numSamples;             // Get starting point after current write position

    // Draw the waveform point by point
    for (int i = 0; i < width; ++i) {
        // Calculate the actual sample index with wraparound
        const float index = std::fmod(start + i * step, static_cast<float>(numSamples));
        // Get the audio sample value at this index
        const float sample = buffer.getSample(0, static_cast<int>(index));
        // Map the sample value (-1 to 1) to screen coordinates (height to 0)
        const float y = juce::jmap(sample, -1.0f, 1.0f, static_cast<float>(height), 0.0f);
        waveformPath.lineTo(static_cast<float>(i), y);
    }

    g.strokePath(waveformPath, juce::PathStrokeType(1.0f)); // Draw the final path
}