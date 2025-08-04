#pragma once

#include <JuceHeader.h>

/**
 * @brief Image names
 * Enum for easy access images
 */
enum imageNames {
    microphone,
    save,
    openFile,
    speaker,
    settings,
    play,
    pause,
    replay,
    arrowDown,
    highpass,
    lowpass,
    leftChannel,
    rightChannel,
    logo1,
    logo2,
    notch
};

/**
 * @brief Struct used images for UI
 * 
 */
struct EQImages {
    EQImages(imageNames val) {
        image = val;
    }
    juce::Image getImage();
    int width, height;
private:
    int image;
    juce::String imagePath = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentExecutableFile).getSiblingFile("images").getFullPathName();
};