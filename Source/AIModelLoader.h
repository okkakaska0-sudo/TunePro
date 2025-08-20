#pragma once

#include "JuceHeader.h"
#include <vector>
#include <memory>

class AIModelLoader
{
public:
    // Pitch prediction structure
    struct PitchPrediction
    {
        float frequency = 0.0f;
        float confidence = 0.0f;
        std::vector<float> harmonics;
        float voicing = 0.0f;
        
        PitchPrediction() : harmonics(16, 0.0f) {}
    };
    
    // Synthesis parameters for DDSP
    struct SynthesisParams
    {
        float fundamentalFreq = 440.0f;
        std::vector<float> harmonicAmplitudes;
        float loudness = 0.5f;
        float noisiness = 0.0f;
        
        SynthesisParams() : harmonicAmplitudes(16, 0.0f) {}
    };

    AIModelLoader();
    ~AIModelLoader();

    // Model management
    bool loadModels();
    bool areModelsLoaded() const { return modelsLoaded; }
    void unloadModels();
    
    // CREPE pitch detection simulation
    PitchPrediction predictPitch(const float* audio, int numSamples, double sampleRate);
    
    // DDSP synthesis simulation  
    bool processWithDDSP(const float* input, float* output, int numSamples, 
                        const SynthesisParams& params);
    
    // Model configuration
    void setModelPath(const String& path) { modelPath = path; }
    String getModelPath() const { return modelPath; }
    
    // Processing settings
    void setProcessingBlockSize(int blockSize) { processingBlockSize = blockSize; }
    void setMaxPolyphony(int polyphony) { maxPolyphony = polyphony; }
    
    // Performance monitoring
    float getCPUUsage() const { return cpuUsage; }
    int64 getProcessingTimeMs() const { return processingTimeMs; }

private:
    // Model state
    bool modelsLoaded = false;
    String modelPath;
    
    // Processing parameters
    int processingBlockSize = 512;
    int maxPolyphony = 1;
    double currentSampleRate = 44100.0;
    
    // Performance monitoring
    float cpuUsage = 0.0f;
    int64 processingTimeMs = 0;
    Time lastProcessTime;
    
    // Audio processing buffers
    AudioBuffer<float> processBuffer;
    AudioBuffer<float> analysisBuffer;
    std::vector<float> windowBuffer;
    
    // Pitch tracking state
    std::vector<float> pitchHistory;
    float lastPitchEstimate = 0.0f;
    float pitchSmoothing = 0.1f;
    
    // Harmonic analysis
    std::unique_ptr<dsp::FFT> fft;
    HeapBlock<dsp::Complex<float>> frequencyData;
    static constexpr int fftOrder = 11; // 2048 samples
    static constexpr int fftSize = 1 << fftOrder;
    
    // DDSP synthesis components
    struct DDSPSynthesizer
    {
        // Harmonic oscillator
        std::vector<float> harmonicPhases;
        std::vector<float> harmonicFreqs;
        std::vector<float> harmonicAmps;
        
        // Noise generator
        Random noiseGenerator;
        std::vector<float> noiseFilter;
        
        // Reverb/filtering
        AudioBuffer<float> reverbBuffer;
        int reverbPosition = 0;
        
        DDSPSynthesizer() : harmonicPhases(16, 0.0f), harmonicFreqs(16, 0.0f), 
                           harmonicAmps(16, 0.0f), noiseFilter(512, 0.0f) {}
    };
    
    std::unique_ptr<DDSPSynthesizer> synthesizer;
    
    // Advanced pitch detection methods
    float detectPitchCREPESimulation(const float* audio, int numSamples);
    PitchPrediction analyzePitchFeatures(const float* audio, int numSamples);
    
    // Spectral analysis
    void performSpectralAnalysis(const float* input, int numSamples, 
                                std::vector<float>& spectrum);
    void extractHarmonics(const std::vector<float>& spectrum, float fundamentalFreq,
                         std::vector<float>& harmonics);
    
    // DDSP synthesis methods
    void synthesizeHarmonics(float* output, int numSamples, const SynthesisParams& params);
    void synthesizeNoise(float* output, int numSamples, float noisiness);
    void applyFormantFiltering(float* audio, int numSamples, float fundamentalFreq);
    
    // Utility methods
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void updatePerformanceMetrics();
    float smoothPitchEstimate(float newPitch);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AIModelLoader)
};
