#pragma once

#include <JuceHeader.h>

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

struct EQImages {
    EQImages(imageNames val) {
        image = val;
    }
    juce::Image getImage();
private:
    int image;
    juce::String imagePath = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentExecutableFile).getSiblingFile("images").getFullPathName();
};