#include "ResponseCurveComponent.hpp"

/**
 * @brief Construct a new Left Response Curve Component:: Left Response Curve Component object
 * Set timer and listener for left response graph
 * @param p 
 */
LeftResponseCurveComponent::LeftResponseCurveComponent(EQlibriumAudioProcessor& p) :
audioProcessor(p) {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->addListener(this);
    }
    updateChain();
    startTimerHz(30);
}

/**
 * @brief Destroy the Left Response Curve Component:: Left Response Curve Component object
 * Remove left listener
 */
LeftResponseCurveComponent::~LeftResponseCurveComponent() {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->removeListener(this);
    }
}

/**
 * @brief Parameter changed
 * For listener left
 * @param parameterIndex 
 * @param newValue 
 */
void LeftResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue) {
    parametersChanged.set(true);
}

/**
 * @brief Timer callback
 * Update filter
 * Repaint left graph
 */
void LeftResponseCurveComponent::timerCallback() {
    auto fftBounds = getLocalBounds().toFloat();
    auto sampleRate = audioProcessor.getSampleRate();
    if(parametersChanged.compareAndSetBool(false, true)) {
        updateChain();
    }
    repaint();
}

/**
 * @brief Update
 * Update left filters
 */
void LeftResponseCurveComponent::updateChain() {
    auto chainSettings = getChainSettings(audioProcessor.apvts);
    auto leftPeakCoefficients = makeLeftPeakFilter(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<Peak>().coefficients, leftPeakCoefficients);
    auto leftLowCutCoefficients = makeLeftLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto leftHighCutCoefficients = makeLeftHighCutFilter(chainSettings, audioProcessor.getSampleRate());
    updateCutFilter(monoChain.get<LowCut>(), leftLowCutCoefficients, chainSettings.leftLowCutSlope);
    updateCutFilter(monoChain.get<HighCut>(), leftHighCutCoefficients, chainSettings.leftHighCutSlope);
}

/**
 * @brief Paint
 * Paint left filter
 * @param g 
 */
void LeftResponseCurveComponent::paint(juce::Graphics& g) {
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
    Colour visualLineColour = Colour(51,51,255);
    Path responseCurve;
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax] (double input) {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    for(size_t i = 1; i < mags.size(); i++)
    {
        responseCurve.lineTo(responseArea.getX()+i, map(mags[i]));
    }
    g.setColour(visualLineColour);
    g.strokePath(responseCurve, PathStrokeType(2.f));
}

/**
 * @brief Resize
 * Resize elemenets before paint
 */
void LeftResponseCurveComponent::resized() {
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
    Array<float> gain {
        -12, 0, 12, 24
    };
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
    for(auto gDb : gain) {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        String str;
        if(gDb > 0) {
            str << "+";
        }
        if(gDb == 0) {
            str << "+-";
        }
        str << gDb;
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(2);
        r.setCentre(r.getCentreX(), y);
        g.setColour(gDb >= 0.f ? Colours::green : Colours::red);
        g.drawFittedText(str, r, Justification::centred, 1);
    }
}

/**
 * @brief Render area
 * (Removable)
 * @return juce::Rectangle<int> 
 */
juce::Rectangle<int> LeftResponseCurveComponent::getRenderArea() {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(0);
    bounds.removeFromBottom(0);
    bounds.removeFromLeft(0);
    bounds.removeFromRight(0);
    return bounds;
}

/**
 * @brief Analysis area
 * Resize default draw space left
 * @return juce::Rectangle<int> 
 */
juce::Rectangle<int> LeftResponseCurveComponent::getAnalysisArea() {
    auto bounds = getRenderArea();
    bounds.removeFromLeft(19);
    bounds.removeFromRight(25);
    bounds.removeFromTop(14);
    bounds.removeFromBottom(7);
    return bounds;
}

//==============================================================================

/**
 * @brief Construct a new Right Response Curve Component:: Right Response Curve Component object
 * Set timer and listener for right response graph
 * @param p 
 */
RightResponseCurveComponent::RightResponseCurveComponent(EQlibriumAudioProcessor& p) :
audioProcessor(p) {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->addListener(this);
    }
    updateChain();
    startTimerHz(30);
}

/**
 * @brief Destroy the Right Response Curve Component:: Right Response Curve Component object
 * Remove right listener
 */
RightResponseCurveComponent::~RightResponseCurveComponent() {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->removeListener(this);
    }
}

/**
 * @brief Parameter changed
 * For listener right
 * @param parameterIndex 
 * @param newValue 
 */
void RightResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue) {
    parametersChanged.set(true);
}

/**
 * @brief Timer callback
 * Update filter
 * Repaint right graph
 */
void RightResponseCurveComponent::timerCallback() {
    auto fftBounds = getLocalBounds().toFloat();
    auto sampleRate = audioProcessor.getSampleRate();
    if(parametersChanged.compareAndSetBool(false, true)) {
        updateChain();
    }
    repaint();
}

/**
 * @brief Update
 * Update right filters
 */
void RightResponseCurveComponent::updateChain() {
    auto chainSettings = getChainSettings(audioProcessor.apvts);
    auto rightPeakCoefficients = makeRightPeakFilter(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<Peak>().coefficients, rightPeakCoefficients);
    auto rightLowCutCoefficients = makeRightLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto rightHighCutCoefficients = makeRightHighCutFilter(chainSettings, audioProcessor.getSampleRate());
    updateCutFilter(monoChain.get<LowCut>(), rightLowCutCoefficients, chainSettings.rightLowCutSlope);
    updateCutFilter(monoChain.get<HighCut>(), rightHighCutCoefficients, chainSettings.rightHighCutSlope);
}

/**
 * @brief Paint
 * Paint right filter
 * @param g 
 */
void RightResponseCurveComponent::paint(juce::Graphics& g) {
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
    Colour visualLineColour = Colour(51,51,255);
    Path responseCurve;
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax] (double input) {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    for(size_t i = 1; i < mags.size(); i++)
    {
        responseCurve.lineTo(responseArea.getX()+i, map(mags[i]));
    }
    g.setColour(visualLineColour);
    g.strokePath(responseCurve, PathStrokeType(2.f));
}

/**
 * @brief Resize
 * Resize elemenets before paint
 */
void RightResponseCurveComponent::resized() {
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
    Array<float> gain {
        -12, 0, 12, 24
    };
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
    for(auto gDb : gain) {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        String str;
        if(gDb > 0) {
            str << "+";
        }
        if(gDb == 0) {
            str << "+-";
        }
        str << gDb;
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(2);
        r.setCentre(r.getCentreX(), y);
        g.setColour(gDb >= 0.f ? Colours::green : Colours::red);
        g.drawFittedText(str, r, Justification::centred, 1);
    }
}

/**
 * @brief Render area
 * (Removable)
 * @return juce::Rectangle<int> 
 */
juce::Rectangle<int> RightResponseCurveComponent::getRenderArea() {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(0);
    bounds.removeFromBottom(0);
    bounds.removeFromLeft(0);
    bounds.removeFromRight(0);
    return bounds;
}

/**
 * @brief Analysis area
 * Resize default draw space right
 * @return juce::Rectangle<int> 
 */
juce::Rectangle<int> RightResponseCurveComponent::getAnalysisArea() {
    auto bounds = getRenderArea();
    bounds.removeFromLeft(19);
    bounds.removeFromRight(25);
    bounds.removeFromTop(14);
    bounds.removeFromBottom(7);
    return bounds;
}