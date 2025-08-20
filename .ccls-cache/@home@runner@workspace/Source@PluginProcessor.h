#pragma once

#include "JuceHeader.h"
#include <memory>
#include "PitchCorrectionEngine.h"
#include "Parameters.h"
#include "PresetManager.h"
#include "ModeSelector.h"
#include "AIModelLoader.h"

#ifdef USE_RUBBERBAND
#include <rubberband/RubberBandStretcher.h>
#endif

class AutoTuneAudioProcessor : public AudioProcessor,
                                public AudioProcessorValueTreeState::Listener
{
public:
    AutoTuneAudioProcessor();
    ~AutoTuneAudioProcessor() override;

    // AudioProcessor interface
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const AudioProcessor::BusesLayout& layouts) const override;
#endif

    void processBlock(AudioBuffer<float>&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override { ignoreUnused(index); }
    const String getProgramName(int index) override;
    void changeProgramName(int index, const String& newName) override;

    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Parameter listener
    void parameterChanged(const String& parameterID, float newValue) override;

    // Public accessors
    AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    Parameters& getParameters() { return pluginParameters; }
    PresetManager& getPresetManager() { return presetManager; }
    PitchCorrectionEngine& getPitchEngine() { return pitchEngine; }
    AIModelLoader& getAIModelLoader() { return aiModelLoader; }

private:
    // Core components - ORDER MATTERS for initialization!
    Parameters pluginParameters;                       // Must be initialized BEFORE parameters
    AudioProcessorValueTreeState parameters;
    PresetManager presetManager;
    PitchCorrectionEngine pitchEngine;
    ModeSelector modeSelector;
    AIModelLoader aiModelLoader;

    // Audio processing variables
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    
    // Pitch detection buffers
    AudioBuffer<float> pitchBuffer;
    AudioBuffer<float> correctedBuffer;
    
    // Circular buffer for overlap-add processing
    AudioBuffer<float> overlapBuffer;
    int overlapPosition = 0;
    static constexpr int overlapSize = 2048;

    // FFT for frequency domain processing
    std::unique_ptr<dsp::FFT> fft;
    std::unique_ptr<dsp::WindowingFunction<float>> window;
    AudioBuffer<float> fftBuffer;
    HeapBlock<dsp::Complex<float>> frequencyData;
    static constexpr int fftOrder = 11; // 2^11 = 2048
    static constexpr int fftSize = 1 << fftOrder;

    // Smoothing filters for parameters
    SmoothedValue<float> speedSmoothed;
    SmoothedValue<float> amountSmoothed;

#ifdef USE_RUBBERBAND
    std::unique_ptr<RubberBand::RubberBandStretcher> rubberBand;
#endif

    // Processing methods
    void processClassicMode(AudioBuffer<float>& buffer);
    void processHardMode(AudioBuffer<float>& buffer);
    void processAIMode(AudioBuffer<float>& buffer);
    
    void performPitchCorrection(AudioBuffer<float>& buffer, 
                               float speed, float amount, 
                               Parameters::Key key, Parameters::Scale scale);
    
    void applyOverlapAdd(AudioBuffer<float>& buffer, 
                        const AudioBuffer<float>& processedBuffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutoTuneAudioProcessor)
};
