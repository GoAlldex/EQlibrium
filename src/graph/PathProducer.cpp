#include "PathProducer.hpp"

/**
 * @brief Process
 * Set draw path for analyser
 * @param fftBounds 
 * @param sampleRate 
 */
void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate) {
    juce::AudioBuffer<float> tempIncomingBuffer;
    while(ChannelFifo->getNumCompleteBuffersAvailable() > 0) {
        if(ChannelFifo->getAudioBuffer(tempIncomingBuffer)) {
            auto size = tempIncomingBuffer.getNumSamples();
            juce::FloatVectorOperations::copy(
                monoBuffer.getWritePointer(0, 0),
                monoBuffer.getReadPointer(0, size),
                monoBuffer.getNumSamples()-size
            );
            juce::FloatVectorOperations::copy(
                monoBuffer.getWritePointer(0, monoBuffer.getNumSamples()-size),
                tempIncomingBuffer.getReadPointer(0, 0),
                size
            );
            ChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
        }
    }
    const auto fftSize = ChannelFFTDataGenerator.getFFTSize();
    const auto binWidth = sampleRate/(double)fftSize;
    while(ChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0) {
        std::vector<float> fftData;
        if(ChannelFFTDataGenerator.getFFTData(fftData)) {
            pathProducer.generatePath(
                fftData,
                fftBounds,
                fftSize,
                binWidth,
                -48.f
            );
        }
    }
    while(pathProducer.getNumPathsAvailable()) {
        pathProducer.getPath(ChannelFFTPath);
    }
}

//==============================================================================

/**
 * @brief Construct a new Left Path Producer Component:: Left Path Producer Component object
 * Set timer and listener
 * @param p 
 */
PathProducerComponent::PathProducerComponent(EQlibriumAudioProcessor& p, int chval) :
audioProcessor(p),
pathProducer(chval == 0 ? audioProcessor.leftChannelFifo : audioProcessor.rightChannelFifo) {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->addListener(this);
    }
    startTimerHz(60);
}

/**
 * @brief Destroy the Left Path Producer
 * Remove listener 
 */
PathProducerComponent::~PathProducerComponent() {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->removeListener(this);
    }
}

/**
 * @brief Paramater changed
 * For listener
 * @param parameterIndex 
 * @param newValue 
 */
void PathProducerComponent::parameterValueChanged(int parameterIndex, float newValue) {
    parametersChanged.set(true);
}

/**
 * @brief Timer callback
 * Repiant analyser
 */
void PathProducerComponent::timerCallback() {
    auto fftBounds = getLocalBounds().toFloat();
    auto sampleRate = audioProcessor.getSampleRate();
    pathProducer.process(fftBounds, sampleRate);
    repaint();
}

/**
 * @brief Paint
 * Paint analyser path
 * @param g 
 */
void PathProducerComponent::paint(juce::Graphics& g) {
    using namespace juce;
    g.fillAll(Colour(50,50,50));
    auto responseArea = getLocalBounds();
    auto w = responseArea.getWidth();
    auto h = responseArea.getHeight();
    auto& lowcut = monoChain.get<LowCut>();
    auto& peak = monoChain.get<Peak>();
    auto& highcut = monoChain.get<HighCut>();
    auto sampleRate = audioProcessor.getSampleRate();
    std::vector<double> mags;
    mags.resize(w);
    for(int i = 0; i < w; i++) {
        double mag = 1.f;
        auto freq = mapToLog10(double(i)/double(w), 20.0, 20000.0);
        if(!monoChain.isBypassed<Peak>()) {
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(!lowcut.isBypassed<0>()) {
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(!lowcut.isBypassed<1>()) {
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(!lowcut.isBypassed<2>()) {
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(!lowcut.isBypassed<3>()) {
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(!highcut.isBypassed<0>()) {
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(!highcut.isBypassed<1>()) {
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(!highcut.isBypassed<2>()) {
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(!highcut.isBypassed<3>()) {
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        mags[i] = Decibels::gainToDecibels(mag);
    }
    g.drawImage(background, getLocalBounds().toFloat());
    using namespace juce;
    Colour visualLineColour = Colour(51,51,255);
    auto rightChannelFFTPath = pathProducer.getPath();
    rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
    g.setColour(visualLineColour);
    g.strokePath(rightChannelFFTPath, PathStrokeType(2.f));
}

/**
 * @brief Resize
 * Resize analyser elements
 */
void PathProducerComponent::resized() {
    using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
    Graphics g(background);
    Colour bgColour = Colour(25,25,25);
    g.setColour(bgColour);
    g.fillRect(background.getBounds());
    Array<float> freqs {
        20, 50, 100,
        200, 500, 1000,
        2000, 5000, 10000,
        20000
    };
    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();
    Array<float> xs;
    for(auto f : freqs) {
        auto normX = mapFromLog10(f, 20.f, 20000.f);
        xs.add(left+width*normX);
    }
    g.setColour(Colours::white);
    const int fontHeight = 14;
    g.setFont(fontHeight);
    for(int i = 0; i < freqs.size(); i++) {
        auto f = freqs[i];
        auto x = xs[i];
        bool addK = false;
        String str;
        if (f > 999.f) {
            addK = true;
            f /= 1000.f;
        }
        str << f;
        if(addK) {
            str << "k";
        }
        str << "Hz";
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(renderArea.getHeight()-2);
        g.drawFittedText(str, r, Justification::centred, 1);
    }
}

/**
 * @brief Render area
 * (Removable)
 * @return juce::Rectangle<int> 
 */
juce::Rectangle<int> PathProducerComponent::getRenderArea() {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(0);
    bounds.removeFromBottom(0);
    bounds.removeFromLeft(0);
    bounds.removeFromRight(0);
    return bounds;
}

/**
 * @brief Render analyser area
 * Smaller box
 * @return juce::Rectangle<int> 
 */
juce::Rectangle<int> PathProducerComponent::getAnalysisArea() {
    auto bounds = getRenderArea();
    bounds.removeFromLeft(19);
    bounds.removeFromRight(25);
    bounds.removeFromTop(14);
    bounds.removeFromBottom(7);
    return bounds;
}