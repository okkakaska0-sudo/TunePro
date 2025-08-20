#pragma once

#include "JuceHeader.h"
#include <vector>
#include <memory>

class PitchCorrectionEngine
{
public:
    PitchCorrectionEngine();
    ~PitchCorrectionEngine();

    // Initialization
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void reset();

    // Pitch detection methods
    void detectPitch(const float* input, int numSamples, float* pitchOutput);
    void detectPitchAdvanced(const float* input, int numSamples, float* pitchOutput);
    
    // Pitch correction methods
    void correctPitch(float* audio, int numSamples, float targetFreq, float speed, float amount);
    void correctPitchHard(float* audio, int numSamples, float targetFreq, float speed, float amount);
    void correctPitchAI(float* audio, int numSamples, float targetFreq, float speed, float amount);
    
    // Analysis methods
    float calculateRMS(const float* buffer, int numSamples);
    void performFFT(const float* input, std::vector<float>& magnitudeOutput);
    void performIFFT(const std::vector<float>& magnitudeInput, float* output);

private:
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    
    // Pitch detection buffers
    AudioBuffer<float> analysisBuffer;
    AudioBuffer<float> correlationBuffer;
    std::vector<float> windowBuffer;
    
    // FFT processing
    std::unique_ptr<dsp::FFT> fft;
    std::unique_ptr<dsp::WindowingFunction<float>> window;
    HeapBlock<dsp::Complex<float>> frequencyData;
    static constexpr int fftOrder = 11; // 2048 samples
    static constexpr int fftSize = 1 << fftOrder;
    
    // Granular synthesis for pitch shifting
    struct GrainData
    {
        float* buffer;
        int size;
        int position;
        float phase;
        float amplitude;
        bool active;
        
        GrainData() : buffer(nullptr), size(0), position(0), phase(0.0f), amplitude(0.0f), active(false) {}
    };
    
    std::vector<GrainData> grains;
    static constexpr int maxGrains = 8;
    static constexpr int grainSize = 1024;
    AudioBuffer<float> grainBuffers;
    int currentGrain = 0;
    
    // Overlap-add processing
    AudioBuffer<float> overlapBuffer;
    int overlapPosition = 0;
    static constexpr int overlapSize = 2048;
    
    // Pitch detection methods
    float detectPitchAutocorrelation(const float* input, int numSamples);
    float detectPitchYIN(const float* input, int numSamples);
    float detectPitchSpectral(const float* input, int numSamples);
    
    // Pitch shifting methods
    void pitchShiftPSOLA(float* audio, int numSamples, float pitchRatio);
    void pitchShiftGranular(float* audio, int numSamples, float pitchRatio);
    void pitchShiftSpectral(float* audio, int numSamples, float pitchRatio);
    
    // Formant preservation
    void preserveFormants(float* audio, int numSamples, float pitchRatio);
    void extractFormantEnvelope(const float* input, int numSamples, std::vector<float>& formants);
    void applyFormantEnvelope(float* audio, int numSamples, const std::vector<float>& formants);
    
    // Utility methods
    void initializeGrains();
    void releaseGrains();
    GrainData* getNextGrain();
    void processGrain(GrainData& grain, float pitchRatio, float speed);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchCorrectionEngine)
};
