## Documentation

# Architecture overview
```PluginProcessor
classDiagram
EQlibriumAudioProcessor <|-- SingleChannelSampleFifo
EQlibriumAudioProcessor <|-- ChainSettings
SingleChannelSampleFifo <|-- Fifo
EQlibriumAudioProcessor : +AudioProcessorValueTreeState apvts
EQlibriumAudioProcessor : +using BlockType
EQlibriumAudioProcessor : +SingleChannelSampleFifo<BlockType> leftChannelFifo
EQlibriumAudioProcessor : +SingleChannelSampleFifo<BlockType> rightChannelFifo
EQlibriumAudioProcessor : +AudioFormatManager formatManager
EQlibriumAudioProcessor : +AudioFormatManager formatManager
EQlibriumAudioProcessor : +unique_ptr<juce::AudioFormatReaderSource> playSource
EQlibriumAudioProcessor : +AudioThumbnailCache thumbnailCache
EQlibriumAudioProcessor : +AudioThumbnail thumbnail
EQlibriumAudioProcessor : +bool writeFile = false;
EQlibriumAudioProcessor : +unique_ptr<juce::AudioFormatWriter> writer
EQlibriumAudioProcessor : +WavAudioFormat writerformat
EQlibriumAudioProcessor : +File file
EQlibriumAudioProcessor : -LinearSmoothedValue<float> rmsLevelLeft
EQlibriumAudioProcessor : -LinearSmoothedValue<float> rmsLevelRight
EQlibriumAudioProcessor : -ChainSettings previousChainSettings
+void prepareToPlay(double sampleRate, int samplesPerBlock)
+void releaseResources()
+bool isBusesLayoutSupported(const BusesLayout& layouts)
+void processBlock(AudioBuffer<float>&, juce::MidiBuffer&)
+AudioProcessorEditor* createEditor()
+bool hasEditor()
+const String getName()
+bool acceptsMidi()
+bool producesMidi()
+bool isMidiEffect()
+double getTailLengthSeconds()
+void getStateInformation (juce::MemoryBlock& destData)
+void setStateInformation (const void* data, int sizeInBytes)
+static AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
+void getFile()
+void loop()
+void play()
+float getRmsValue(const int channel)
+void smoothLoudness(juce::AudioBuffer<float>& buffer)
+AudioThumbnail* getThumbnail()
+void recordVoice(juce::AudioBuffer<float>& buffer)
+void prepareRecord()
+void prepareSamples()
-void updatePeakFilter(const ChainSettings& chainSettings)
-void updateLowCutFilters(const ChainSettings& chainSettings);
-void updateHighCutFilters(const ChainSettings& chainSettings)
-void updateFilters()

class SingleChannelSampleFifo {

}

class ChainSettings {

}

class Fifo {

}

# Signal processing

# GUI


# Signal flow


# Special features
- Custom VU-Meter with delay (high to low dB)
- Custom filter graph
- Custom frequency graph
- All buttons with own LookAndFeel
- No images from external resources