
#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

void LookAndFeel::drawRotarySlider(
    juce::Graphics & g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider & slider) {
    using namespace juce;
    Colour rotaryBGColour = Colour(25,25,25);
    Colour rotaryPosColour = Colour(51,51,255);
    auto bounds = Rectangle<float>(x, y, width, height);
    g.setColour(rotaryBGColour);
    g.fillEllipse(bounds);
    g.setColour(Colours::black);
    g.drawEllipse(bounds, 0.1f);
    if(auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)) {
        g.setColour(rotaryPosColour);
        auto center = bounds.getCentre();
        Path p;
        Rectangle<float> r;
        r.setLeft(center.getX()-2);
        r.setRight(center.getX()+2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY()-rswl->getTextHeight()*1.5);
        p.addRoundedRectangle(r, 2.f);
        p.addRectangle(r);
        jassert(rotaryStartAngle < rotaryEndAngle);
        auto sliderAngRad = jmap(
            sliderPosProportional,
            0.f,
            1.f,
            rotaryStartAngle,
            rotaryEndAngle
        );
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
        g.fillPath(p);
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        r.setSize(strWidth+4, rswl->getTextHeight()+2);
        r.setCentre(bounds.getCentre());
        g.setColour(rotaryBGColour);
        g.fillRect(r);
        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}


//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics &g){
    using namespace juce;
    auto startAng = degreesToRadians(180.f+45.f);
    auto endAng = degreesToRadians(180.f-45.f)+MathConstants<float>::twoPi;
    auto range = getRange();
    auto sliderBounds = getSliderBounds();
    /*g.setColour(Colours::black);
    g.drawRect(getLocalBounds().toFloat(), 0.3f);*/
    getLookAndFeel().drawRotarySlider(
        g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        jmap(
            getValue(),
            range.getStart(),
            range.getEnd(),
            0.0,
            1.0
        ),
        startAng,
        endAng,
        *this
    );
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth()*0.5f;
    g.setColour(Colours::white);
    g.setFont(getTextHeight());
    auto numChoices = labels.size();
    for(int i = 0; i < numChoices; i++) {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        auto ang = jmap(
            pos,
            0.f,
            1.f,
            startAng,
            endAng
        );
        auto c = center.getPointOnCircumference(radius+getTextHeight()*0.5f+1, ang);
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY()+getTextHeight());
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const {
    auto bounds = getLocalBounds();
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    size -= getTextHeight()*2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const {
    if(auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param)) {
        return choiceParam->getCurrentChoiceName();
    }
    juce::String str;
    bool addK = false;
    if(auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param)) {
        float val = getValue();
        if(val > 999.f) {
            val /= 1000.f;
            addK = true;
        }
        str = juce::String(val, (addK ? 2 : 0));
    } else {
        jassertfalse;
    }
    if(suffix.isNotEmpty()) {
        str << " ";
        if(addK) {
            str << "k";
        }
        str << suffix;
    }
    return str;
}

//==============================================================================
ResponseCurveComponent::ResponseCurveComponent(EQlibriumAudioProcessor& p) :
audioProcessor(p),
leftPathProducer(audioProcessor.leftChannelFifo),
rightPathProducer(audioProcessor.rightChannelFifo) {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->addListener(this);
    }
    updateChain();
    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent() {
    const auto& params = audioProcessor.getParameters();
    for(auto param : params) {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue) {
    parametersChanged.set(true);
}

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

void ResponseCurveComponent::timerCallback() {
    auto fftBounds = getLocalBounds().toFloat();
    auto sampleRate = audioProcessor.getSampleRate();
    leftPathProducer.process(fftBounds, sampleRate);
    rightPathProducer.process(fftBounds, sampleRate);
    if(parametersChanged.compareAndSetBool(false, true)) {
        updateChain();
    }
    repaint();
}

void ResponseCurveComponent::updateChain() {
    auto chainSettings = getChainSettings(audioProcessor.apvts);
    auto leftPeakCoefficients = makeLeftPeakFilter(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<Peak>().coefficients, leftPeakCoefficients);
    auto rightPeakCoefficients = makeRightPeakFilter(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<Peak>().coefficients, rightPeakCoefficients);
    auto leftLowCutCoefficients = makeLeftLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto leftHighCutCoefficients = makeLeftHighCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto rightLowCutCoefficients = makeRightLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto rightHighCutCoefficients = makeRightHighCutFilter(chainSettings, audioProcessor.getSampleRate());
    updateCutFilter(monoChain.get<LowCut>(), leftLowCutCoefficients, chainSettings.leftLowCutSlope);
    updateCutFilter(monoChain.get<HighCut>(), leftHighCutCoefficients, chainSettings.leftHighCutSlope);
    updateCutFilter(monoChain.get<LowCut>(), rightLowCutCoefficients, chainSettings.rightLowCutSlope);
    updateCutFilter(monoChain.get<HighCut>(), rightHighCutCoefficients, chainSettings.rightHighCutSlope);
}

void ResponseCurveComponent::paintLeftFilter(juce::Graphics& g, juce::Rectangle<int> &responseArea, int w, int h, std::vector<double> &mags) {
    using namespace juce;
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

void ResponseCurveComponent::paintRightFilter(juce::Graphics& g, juce::Rectangle<int> &responseArea, int w, int h, std::vector<double> &mags) {
    using namespace juce;
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

void ResponseCurveComponent::paintLeftFreq(juce::Graphics& g, juce::Rectangle<int> &responseArea, int w, int h, std::vector<double> &mags) {
    using namespace juce;
    Colour visualLineColour = Colour(255,255,255);
    auto leftChannelFFTPath = leftPathProducer.getPath();
    leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
    g.setColour(visualLineColour);
    g.strokePath(leftChannelFFTPath, PathStrokeType(2.f));
}

void ResponseCurveComponent::paintRightFreq(juce::Graphics& g, juce::Rectangle<int> &responseArea, int w, int h, std::vector<double> &mags) {
    using namespace juce;
    Colour visualLineColour = Colour(51,51,255);
    auto rightChannelFFTPath = rightPathProducer.getPath();
    rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
    g.setColour(visualLineColour);
    g.strokePath(rightChannelFFTPath, PathStrokeType(2.f));
}

/*void ResponseCurveComponent::paintFreq(juce::Graphics& g, juce::Rectangle<int> &responseArea, int w, int h, std::vector<double> &mags) {
    using namespace juce;
    Colour visualLineColour = Colour(51,51,255);
    g.setColour(visualLineColour);
    if(curveComponent == 1) {
        auto leftChannelFFTPath = leftPathProducer.getPath();
        leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
        g.strokePath(leftChannelFFTPath, PathStrokeType(2.f));
    } else {
        g.setColour(Colours::white);
        auto rightChannelFFTPath = rightPathProducer.getPath();
        rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
        g.strokePath(rightChannelFFTPath, PathStrokeType(2.f));
    }
}*/

void ResponseCurveComponent::paint(juce::Graphics& g) {
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
    switch(curveComponent) {
        default:
        break;
        case 0:
            paintLeftFilter(g, responseArea, w, h, mags);
        break;
        case 1:
            //paintFreq(g, responseArea, w, h, mags);
            paintLeftFreq(g, responseArea, w, h, mags);
        break;
        case 2:
            paintRightFilter(g, responseArea, w, h, mags);
        break;
        case 3:
            //paintFreq(g, responseArea, w, h, mags);
            paintRightFreq(g, responseArea, w, h, mags);
        break;
    }
}

void ResponseCurveComponent::resized() {
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

juce::Rectangle<int> ResponseCurveComponent::getRenderArea() {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(0);
    bounds.removeFromBottom(0);
    bounds.removeFromLeft(0);
    bounds.removeFromRight(0);
    return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea() {
    auto bounds = getRenderArea();
    bounds.removeFromLeft(19);
    bounds.removeFromRight(25);
    bounds.removeFromTop(14);
    bounds.removeFromBottom(7);
    return bounds;
}

//==============================================================================
EQlibriumAudioProcessorEditor::EQlibriumAudioProcessorEditor (EQlibriumAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    peakFreqSliderLeft(*audioProcessor.apvts.getParameter("Left Peak Freq"), "Hz"),
    peakGainSliderLeft(*audioProcessor.apvts.getParameter("Left Peak Gain"), "dB"),
    peakQualitySliderLeft(*audioProcessor.apvts.getParameter("Left Peak Quality"), ""),
    lowCutFreqSliderLeft(*audioProcessor.apvts.getParameter("Left LowCut Freq"), "Hz"),
    lowCutSlopeSliderLeft(*audioProcessor.apvts.getParameter("Left LowCut Slope"), "dB/Okt"),
    highCutFreqSliderLeft(*audioProcessor.apvts.getParameter("Left HighCut Freq"), "Hz"),
    highCutSlopeSliderLeft(*audioProcessor.apvts.getParameter("Left HighCut Slope"), "dB/Okt"),
    peakFreqSliderRight(*audioProcessor.apvts.getParameter("Right Peak Freq"), "Hz"),
    peakGainSliderRight(*audioProcessor.apvts.getParameter("Right Peak Gain"), "dB"),
    peakQualitySliderRight(*audioProcessor.apvts.getParameter("Right Peak Quality"), ""),
    lowCutFreqSliderRight(*audioProcessor.apvts.getParameter("Right LowCut Freq"), "Hz"),
    lowCutSlopeSliderRight(*audioProcessor.apvts.getParameter("Right LowCut Slope"), "dB/Okt"),
    highCutFreqSliderRight(*audioProcessor.apvts.getParameter("Right HighCut Freq"), "Hz"),
    highCutSlopeSliderRight(*audioProcessor.apvts.getParameter("Right HighCut Slope"), "dB/Okt"),
    filterLeft(audioProcessor),
    freqLeft(audioProcessor),
    leftPeakFreqSliderAttachment(audioProcessor.apvts, "Left Peak Freq", peakFreqSliderLeft),
    leftPeakGainSliderAttachment(audioProcessor.apvts, "Left Peak Gain", peakGainSliderLeft),
    leftPeakQualitySliderAttachment(audioProcessor.apvts, "Left Peak Quality", peakQualitySliderLeft),
    leftLowCutFreqSliderAttachment(audioProcessor.apvts, "Left LowCut Freq", lowCutFreqSliderLeft),
    leftLowCutSlopeSliderAttachment(audioProcessor.apvts, "Left LowCut Slope", lowCutSlopeSliderLeft),
    leftHighCutFreqSliderAttachment(audioProcessor.apvts, "Left HighCut Freq", highCutFreqSliderLeft),
    leftHighCutSlopeSliderAttachment(audioProcessor.apvts, "Left HighCut Slope", highCutSlopeSliderLeft),
    rightPeakFreqSliderAttachment(audioProcessor.apvts, "Right Peak Freq", peakFreqSliderRight),
    rightPeakGainSliderAttachment(audioProcessor.apvts, "Right Peak Gain", peakGainSliderRight),
    rightPeakQualitySliderAttachment(audioProcessor.apvts, "Right Peak Quality", peakQualitySliderRight),
    rightLowCutFreqSliderAttachment(audioProcessor.apvts, "Right LowCut Freq", lowCutFreqSliderRight),
    rightLowCutSlopeSliderAttachment(audioProcessor.apvts, "Right LowCut Slope", lowCutSlopeSliderRight),
    rightHighCutFreqSliderAttachment(audioProcessor.apvts, "Right HighCut Freq", highCutFreqSliderRight),
    rightHighCutSlopeSliderAttachment(audioProcessor.apvts, "Right HighCut Slope", highCutSlopeSliderRight),
    filterRight(audioProcessor),
    freqRight(audioProcessor) {
    peakFreqSliderLeft.labels.add({0.f, "20Hz"});
    peakFreqSliderLeft.labels.add({1.f, "20kHz"});
    peakGainSliderLeft.labels.add({0.f, "-24dB"});
    peakGainSliderLeft.labels.add({1.f, "+24dB"});
    peakQualitySliderLeft.labels.add({0.f, "0.1"});
    peakQualitySliderLeft.labels.add({1.f, "10"});
    lowCutFreqSliderLeft.labels.add({0.f, "20Hz"});
    lowCutFreqSliderLeft.labels.add({1.f, "20kHz"});
    highCutFreqSliderLeft.labels.add({0.f, "20Hz"});
    highCutFreqSliderLeft.labels.add({1.f, "20kHz"});
    lowCutSlopeSliderLeft.labels.add({0.f, "12"});
    lowCutSlopeSliderLeft.labels.add({1.f, "48"});
    highCutSlopeSliderLeft.labels.add({0.f, "12"});
    highCutSlopeSliderLeft.labels.add({1.f, "48"});
    peakFreqSliderRight.labels.add({0.f, "20Hz"});
    peakFreqSliderRight.labels.add({1.f, "20kHz"});
    peakGainSliderRight.labels.add({0.f, "-24dB"});
    peakGainSliderRight.labels.add({1.f, "+24dB"});
    peakQualitySliderRight.labels.add({0.f, "0.1"});
    peakQualitySliderRight.labels.add({1.f, "10"});
    lowCutFreqSliderRight.labels.add({0.f, "20Hz"});
    lowCutFreqSliderRight.labels.add({1.f, "20kHz"});
    highCutFreqSliderRight.labels.add({0.f, "20Hz"});
    highCutFreqSliderRight.labels.add({1.f, "20kHz"});
    lowCutSlopeSliderRight.labels.add({0.f, "12"});
    lowCutSlopeSliderRight.labels.add({1.f, "48"});
    highCutSlopeSliderRight.labels.add({0.f, "12"});
    highCutSlopeSliderRight.labels.add({1.f, "48"});
    for(auto* comp : getComps()) {
        addAndMakeVisible(comp);
    }
    setSize (1000, 1000);
}
EQlibriumAudioProcessorEditor::~EQlibriumAudioProcessorEditor() { }

//==============================================================================
void EQlibriumAudioProcessorEditor::paint (juce::Graphics& g) {
    using namespace juce;
    /*
    // DEBUG LAYOUT
    // Show layout
    g.setColour(Colours::red);
    g.drawRect(window_micro_rect.toFloat(), 0.1f);
    g.setColour(Colours::red);
    g.drawRect(window_settings_rect.toFloat(), 0.1f);
    g.setColour(Colours::red);
    g.drawRect(window_filter_rect.toFloat(), 0.1f);
    g.setColour(Colours::red);
    g.drawRect(window_analyser_rect.toFloat(), 0.1f);
    g.setColour(Colours::red);
    g.drawRect(window_vumeter_rect.toFloat(), 0.1f);
    // Show channel split
    g.setColour(Colours::green);
    g.drawRect(window_filter_left_rect.toFloat(), 0.1f);
    g.drawRect(window_analyser_left_rect.toFloat(), 0.1f);
    g.drawRect(window_vumeter_left_rect.toFloat(), 0.1f);
    g.setColour(Colours::blue);
    g.drawRect(window_filter_right_rect.toFloat(), 0.1f);
    g.drawRect(window_analyser_right_rect.toFloat(), 0.1f);
    g.drawRect(window_vumeter_right_rect.toFloat(), 0.1f);
    // Show graph split
    g.setColour(Colours::white);
    g.drawRect(window_analyser_left_filter_rect.toFloat(), 0.1f);
    g.drawRect(window_analyser_right_filter_rect.toFloat(), 0.1f);
    g.setColour(Colours::white);
    g.drawRect(window_analyser_left_freq_rect.toFloat(), 0.1f);
    g.drawRect(window_analyser_right_freq_rect.toFloat(), 0.1f);*/
    Colour bgColour = Colour(50,50,50);
    Colour lineColour = Colour(25,25,25);
    g.setColour(bgColour);
    g.fillRect(getLocalBounds());
    g.setColour(lineColour);
    g.drawRect(window_micro_rect.toFloat(), 0.3f);
    g.drawRect(window_settings_rect.toFloat(), 0.3f);
    g.drawRect(peakL.toFloat(), 0.3f);
    g.drawRect(peakR.toFloat(), 0.3f);
    g.drawRect(highCutL.toFloat(), 0.3f);
    g.drawRect(highCutR.toFloat(), 0.3f);
    g.drawRect(lowCutL.toFloat(), 0.3f);
    g.drawRect(lowCutR.toFloat(), 0.3f);
    g.fillRect(window_analyser_left_filter_rect);
    g.fillRect(window_analyser_right_filter_rect);
    g.fillRect(window_analyser_left_freq_rect);
    g.fillRect(window_analyser_right_freq_rect);
}

void EQlibriumAudioProcessorEditor::resized() {
    window_full_rect = getLocalBounds();
    auto h = window_full_rect.getHeight();
    // Categorized places
    window_micro_rect = window_full_rect.removeFromTop(h/6);
    window_settings_rect = window_full_rect.removeFromTop(h/10);
    window_filter_rect = window_full_rect.removeFromTop(h*19/30/2);
    window_analyser_rect = window_full_rect.removeFromTop(h*19/30/2);
    window_vumeter_rect = window_full_rect.removeFromTop(h/10);
    // Padding and margin micro rect
    window_micro_rect.removeFromTop(6);
    window_micro_rect.removeFromBottom(3);
    window_micro_rect.removeFromLeft(6);
    window_micro_rect.removeFromRight(6);
    // Padding and margin settings rect
    window_settings_rect.removeFromTop(3);
    window_settings_rect.removeFromBottom(3);
    window_settings_rect.removeFromLeft(6);
    window_settings_rect.removeFromRight(6);
    // L/R channel splitted filter rects
    window_filter_left_rect = window_filter_rect.removeFromLeft(window_filter_rect.getWidth()/2);
    window_filter_right_rect = window_filter_rect.removeFromLeft(window_filter_rect.getWidth());
    // Padding and margin filter rects
    window_filter_left_rect.removeFromTop(3);
    window_filter_left_rect.removeFromBottom(3);
    window_filter_left_rect.removeFromLeft(6);
    window_filter_left_rect.removeFromRight(3);
    window_filter_right_rect.removeFromTop(3);
    window_filter_right_rect.removeFromBottom(3);
    window_filter_right_rect.removeFromLeft(3);
    window_filter_right_rect.removeFromRight(6);
    // L/R channel splitted analyser rects
    window_analyser_left_rect = window_analyser_rect.removeFromLeft(window_analyser_rect.getWidth()/2);
    window_analyser_right_rect = window_analyser_rect.removeFromLeft(window_analyser_rect.getWidth());
    // Padding and margin analyser rects
    window_analyser_left_rect.removeFromTop(3);
    window_analyser_left_rect.removeFromBottom(3);
    window_analyser_left_rect.removeFromLeft(6);
    window_analyser_left_rect.removeFromRight(3);
    window_analyser_right_rect.removeFromTop(3);
    window_analyser_right_rect.removeFromBottom(3);
    window_analyser_right_rect.removeFromLeft(3);
    window_analyser_right_rect.removeFromRight(6);
    // Analyzer split filter and frequency graph
    window_analyser_left_filter_rect = window_analyser_left_rect.removeFromTop(window_analyser_left_rect.getHeight()/2);
    window_analyser_left_filter_rect.removeFromBottom(3);
    window_analyser_left_freq_rect = window_analyser_left_rect.removeFromTop(window_analyser_left_rect.getHeight());
    window_analyser_left_freq_rect.removeFromTop(3);
    window_analyser_right_filter_rect = window_analyser_right_rect.removeFromTop(window_analyser_right_rect.getHeight()/2);
    window_analyser_right_filter_rect.removeFromBottom(3);
    window_analyser_right_freq_rect = window_analyser_right_rect.removeFromTop(window_analyser_right_rect.getHeight());
    window_analyser_right_freq_rect.removeFromTop(3);
    // L/R channel splitted uv-meter rects
    window_vumeter_left_rect = window_vumeter_rect.removeFromLeft(window_vumeter_rect.getWidth()/2);
    window_vumeter_right_rect = window_vumeter_rect.removeFromLeft(window_vumeter_rect.getWidth());
    // Padding and margin uv-meter rects
    window_vumeter_left_rect.removeFromTop(3);
    window_vumeter_left_rect.removeFromBottom(6);
    window_vumeter_left_rect.removeFromLeft(6);
    window_vumeter_left_rect.removeFromRight(3);
    window_vumeter_right_rect.removeFromTop(3);
    window_vumeter_right_rect.removeFromBottom(6);
    window_vumeter_right_rect.removeFromLeft(3);
    window_vumeter_right_rect.removeFromRight(6);
    // Set left
    auto filterLeftH = window_filter_left_rect.getHeight();
    peakL = window_filter_left_rect.removeFromTop(filterLeftH/3);
    peakL.removeFromBottom(3);
    auto peakLD = juce::Rectangle(7, 270, peakL.getWidth()-2, peakL.getHeight()-2);
    auto peakWL = peakLD.getWidth();
    peakFreqSliderLeft.setBounds(peakLD.removeFromLeft(peakWL/3));
    peakGainSliderLeft.setBounds(peakLD.removeFromRight(peakWL/3));
    peakQualitySliderLeft.setBounds(peakLD.removeFromRight(peakWL/3));
    highCutL = window_filter_left_rect.removeFromTop(filterLeftH/3);
    highCutL.removeFromBottom(3);
    auto highCutLD = juce::Rectangle(7, 372, highCutL.getWidth()-2, highCutL.getHeight()-2);
    auto highCutWL = highCutLD.getWidth();
    highCutFreqSliderLeft.setBounds(highCutLD.removeFromLeft(highCutWL/2));
    highCutSlopeSliderLeft.setBounds(highCutLD.removeFromLeft(highCutWL/2));
    lowCutL = window_filter_left_rect.removeFromTop(filterLeftH/3);
    lowCutL.removeFromBottom(3);
    auto lowCutLD = juce::Rectangle(7, 474, lowCutL.getWidth()-2, lowCutL.getHeight()-2);
    auto lowCutWL = lowCutLD.getWidth();
    lowCutFreqSliderLeft.setBounds(lowCutLD.removeFromLeft(lowCutWL/2));
    lowCutSlopeSliderLeft.setBounds(lowCutLD.removeFromLeft(lowCutWL/2));
    filterLeft.setCurveComponent(0);
    filterLeft.setBounds(window_analyser_left_filter_rect);
    freqLeft.setCurveComponent(1);
    freqLeft.setBounds(window_analyser_left_freq_rect);
    //Set right
    auto filterRightH = window_filter_right_rect.getHeight();
    peakR = window_filter_right_rect.removeFromTop(filterRightH/3);
    peakR.removeFromBottom(3);
    auto peakRD = juce::Rectangle(peakR.getWidth()+14, 270, peakR.getWidth()-2, peakR.getHeight()-2);
    auto peakWR = peakRD.getWidth();
    peakFreqSliderRight.setBounds(peakRD.removeFromLeft(peakWR/3));
    peakGainSliderRight.setBounds(peakRD.removeFromRight(peakWR/3));
    peakQualitySliderRight.setBounds(peakRD.removeFromRight(peakWR/3));
    highCutR = window_filter_right_rect.removeFromTop(filterRightH/3);
    highCutR.removeFromBottom(3);
    auto highCutRD = juce::Rectangle(highCutR.getWidth()+14, 372, highCutR.getWidth()-2, highCutR.getHeight()-2);
    auto highCutWR = highCutRD.getWidth();
    highCutFreqSliderRight.setBounds(highCutRD.removeFromLeft(highCutWR/2));
    highCutSlopeSliderRight.setBounds(highCutRD.removeFromLeft(highCutWR/2));
    lowCutR = window_filter_right_rect.removeFromTop(filterRightH/3);
    lowCutR.removeFromBottom(3);
    auto lowCutRD = juce::Rectangle(lowCutR.getWidth()+14, 474, lowCutR.getWidth()-2, lowCutR.getHeight()-2);
    auto lowCutWR = lowCutRD.getWidth();
    lowCutFreqSliderRight.setBounds(lowCutRD.removeFromLeft(lowCutWR/2));
    lowCutSlopeSliderRight.setBounds(lowCutRD.removeFromLeft(lowCutWR/2));
    filterRight.setCurveComponent(2);
    filterRight.setBounds(window_analyser_right_filter_rect);
    freqRight.setCurveComponent(3);
    freqRight.setBounds(window_analyser_right_freq_rect);
}

std::vector<juce::Component*> EQlibriumAudioProcessorEditor::getComps()
{
    return {
        &peakFreqSliderLeft,
        &peakGainSliderLeft,
        &peakQualitySliderLeft,
        &lowCutFreqSliderLeft,
        &highCutFreqSliderLeft,
        &lowCutSlopeSliderLeft,
        &highCutSlopeSliderLeft,
        &filterLeft,
        &freqLeft,
        &peakFreqSliderRight,
        &peakGainSliderRight,
        &peakQualitySliderRight,
        &lowCutFreqSliderRight,
        &highCutFreqSliderRight,
        &lowCutSlopeSliderRight,
        &highCutSlopeSliderRight,
        &filterRight,
        &freqRight
    };
}