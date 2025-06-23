
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
    /*g.setColour(Colours::red);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::yellow);
    g.drawRect(sliderBounds);*/
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
    while(leftChannelFifo->getNumCompleteBuffersAvailable() > 0) {
        if(leftChannelFifo->getAudioBuffer(tempIncomingBuffer)) {
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
            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
        }
    }

    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
    const auto binWidth = sampleRate/(double)fftSize;
    while(leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0) {
        std::vector<float> fftData;
        if(leftChannelFFTDataGenerator.getFFTData(fftData)) {
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
        pathProducer.getPath(leftChannelFFTPath);
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
    auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
    updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
}


void ResponseCurveComponent::paint (juce::Graphics& g) {
    using namespace juce;
    Colour bgColour = Colour(50,50,50);
    Colour visualBGColour = Colour(25,25,25);
    Colour visualLineColour = Colour(51,51,255);
    g.fillAll(bgColour);
    auto responseArea = getLocalBounds();
    //auto responseArea = getRenderArea();
    auto w = responseArea.getWidth();
    auto h = responseArea.getHeight();
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    auto sampleRate = audioProcessor.getSampleRate();
    std::vector<double> mags;
    mags.resize(w);
    for(int i = 0; i < w; i++) {
        double mag = 1.f;
        auto freq = mapToLog10(double(i)/double(w), 20.0, 20000.0);
        if(!monoChain.isBypassed<ChainPositions::Peak>()) {
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
    auto leftChannelFFTPath = leftPathProducer.getPath();
    leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
    auto rightChannelFFTPath = leftPathProducer.getPath();
    rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
    g.setColour(visualBGColour);
    g.fillRect(responseArea.toFloat());
    g.setColour(Colours::black);
    g.drawRect(responseArea.toFloat(), 0.1f);
    g.drawImage(background, getLocalBounds().toFloat());
    g.setColour(Colours::red);
    g.strokePath(leftChannelFFTPath, PathStrokeType(1));
    g.setColour(Colours::yellow);
    g.strokePath(rightChannelFFTPath, PathStrokeType(1));
    g.setColour(visualLineColour);
    g.strokePath(responseCurve, PathStrokeType(2.f));
}

void ResponseCurveComponent::resized() {
    using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
    Graphics g(background);
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
    peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
    peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
    peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
    lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Okt"),
    highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "dB/Okt"),
    responseCurveComponent(audioProcessor),
    peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider) {
    peakFreqSlider.labels.add({0.f, "20Hz"});
    peakFreqSlider.labels.add({1.f, "20kHz"});
    peakGainSlider.labels.add({0.f, "-24dB"});
    peakGainSlider.labels.add({1.f, "+24dB"});
    peakQualitySlider.labels.add({0.f, "0.1"});
    peakQualitySlider.labels.add({1.f, "10"});
    lowCutFreqSlider.labels.add({0.f, "20Hz"});
    lowCutFreqSlider.labels.add({1.f, "20kHz"});
    highCutFreqSlider.labels.add({0.f, "20Hz"});
    highCutFreqSlider.labels.add({1.f, "20kHz"});
    lowCutSlopeSlider.labels.add({0.f, "12"});
    lowCutSlopeSlider.labels.add({1.f, "48"});
    highCutSlopeSlider.labels.add({0.f, "12"});
    highCutSlopeSlider.labels.add({1.f, "48"});
    for(auto* comp : getComps()) {
        addAndMakeVisible(comp);
    }
    setSize (1000, 600);
}
EQlibriumAudioProcessorEditor::~EQlibriumAudioProcessorEditor() { }

//==============================================================================
void EQlibriumAudioProcessorEditor::paint (juce::Graphics& g) {
    // DEBUG LAYOUT
    using namespace juce;
    // Show layout
    g.setColour(Colours::red);
    g.drawRect(window_micro_rect.toFloat(), 1.f);
    g.setColour(Colours::red);
    g.drawRect(window_settings_rect.toFloat(), 1.f);
    g.setColour(Colours::red);
    g.drawRect(window_filter_rect.toFloat(), 1.f);
    g.setColour(Colours::red);
    g.drawRect(window_analyser_rect.toFloat(), 1.f);
    g.setColour(Colours::red);
    g.drawRect(window_vumeter_rect.toFloat(), 1.f);
    // Show channel split
    g.setColour(Colours::green);
    g.drawRect(window_filter_left_rect.toFloat(), 1.f);
    g.drawRect(window_analyser_left_rect.toFloat(), 1.f);
    g.drawRect(window_vumeter_left_rect.toFloat(), 1.f);
    g.setColour(Colours::blue);
    g.drawRect(window_filter_right_rect.toFloat(), 1.f);
    g.drawRect(window_analyser_right_rect.toFloat(), 1.f);
    g.drawRect(window_vumeter_right_rect.toFloat(), 1.f);
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
    /*auto bounds = getLocalBounds();
    float hRatio = 25.f/100.f;
    auto responseArea = bounds.removeFromTop(bounds.getHeight()*hRatio);
    responseCurveComponent.setBounds(responseArea);
    bounds.removeFromTop(5);
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth()*0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth()*0.5);
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight()*0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight()*0.5));
    highCutSlopeSlider.setBounds(highCutArea);
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight()*0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight()*0.5));
    peakQualitySlider.setBounds(bounds);*/
}

std::vector<juce::Component*> EQlibriumAudioProcessorEditor::getComps()
{
    return {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider,
        &responseCurveComponent
    };
}