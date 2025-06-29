#include "EQImages.hpp"

juce::Image EQImages::getImage() {
    juce::Array<juce::String> images;
    images.resize(16);
    images.set(0, "microphone.png");
    images.set(1, "save.png");
    images.set(2, "folder.png");
    images.set(3, "speaker.png");
    images.set(4, "settings.png");
    images.set(5, "play.png");
    images.set(6, "pause.png");
    images.set(7, "replay.png");
    images.set(8, "arrow_down.png");
    images.set(9, "highpass.png");
    images.set(10, "lowpass.png");
    images.set(11, "left_channel.png");
    images.set(12, "right_channel.png");
    images.set(13, "logo1.png");
    images.set(14, "logo2.png");
    images.set(15, "notch.png");
    if(image < images.size()) {
        juce::Image setImage = juce::ImageFileFormat::loadFrom(juce::File(imagePath+"/"+images[image]));
        width = setImage.getWidth();
        height = setImage.getHeight();
        return setImage;
    }
    juce::Image setImage = juce::ImageFileFormat::loadFrom(juce::File(imagePath+"/"+images[0]));
    width = setImage.getWidth();
    height = setImage.getHeight();
    return setImage;
}