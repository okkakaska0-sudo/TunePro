#pragma once

#include "JuceHeader.h"
#include <memory>
#include <vector>
#include <complex>

#ifdef USE_FFTW
#include <fftw3.h>
#endif

#ifdef USE_EIGEN
#include <Eigen/Dense>
#endif

class PitchCorrectionEngine
{
public:
    PitchCorrectionEngine();
    ~PitchCorrectionEngine();

    void prepare(double sampleRate, int blockSize);
    void prepareToPlay(double sampleRate, int blockSize);
    void reset();

    // Basic pitch detection methods
    void detectPitch(const float* inputBuffer, int numSamples, float* pitchOutput);
    void detectPitchAdvanced(const float* inputBuffer, int numSamples, float* pitchOutput);
    
    // Pitch correction methods
    void correctPitch(float* buffer, int numSamples, 
                     float targetPitch, float speed, float amount);
    
    void correctPitchHard(float* buffer, int numSamples, 
                         float targetPitch, float speed, float amount);
    
    void correctPitchAI(float* buffer, int numSamples, 
                       float targetPitch, float speed, float amount);

    // Analysis methods
    float getCurrentPitch() const { return currentPitch; }
    float getCurrentConfidence() const { return pitchConfidence; }
    float getRMSLevel() const { return rmsLevel; }

private:
    double sampleRate = 44100.0;
    int blockSize = 512;
    
    // Current analysis results
    float currentPitch = 0.0f;
    float pitchConfidence = 0.0f;
    float rmsLevel = 0.0f;
    
    // JUCE components for FFT and spectral analysis
    std::unique_ptr<dsp::FFT> fft;
    std::unique_ptr<dsp::WindowingFunction<float>> window;
    HeapBlock<dsp::Complex<float>> frequencyData;
    
    // Autocorrelation-based pitch detection
    std::vector<float> autocorrelationBuffer;
    std::vector<float> windowedBuffer;
    std::vector<float> tempBuffer;
    std::vector<float> magnitudeSpectrum;
    static constexpr int fftOrder = 12; // 2^12 = 4096
    static constexpr int fftSize = 1 << fftOrder;
    
    // Pitch tracking
    std::vector<float> pitchHistory;
    static constexpr int pitchHistoryLength = 10;
    float pitchSmoothingFactor = 0.8f;
    
    // Formant preservation
    std::vector<float> formantFrequencies;
    std::vector<float> formantAmplitudes;
    static constexpr int maxFormants = 5;
    
    // PSOLA (Pitch Synchronous Overlap Add) for pitch shifting
    struct GrainData
    {
        std::vector<float> grain;
        int position;
        float pitch;
        float amplitude;
    };
    
    std::vector<GrainData> grainBuffer;
    int grainSize = 1024;
    int hopSize = 256;
    int grainOverlap = 4;
    
    // Private methods
    float detectPitchAutocorrelation(const float* buffer, int numSamples);
    float detectPitchYIN(const float* buffer, int numSamples);
    float detectPitchSpectral(const float* buffer, int numSamples);
    float detectPitchHarmonic(const float* buffer, int numSamples);
    
    void analyzeSpectrum(const float* buffer, int numSamples);
    void detectFormants(const std::vector<float>& spectrum);
    void preserveFormants(float* buffer, int numSamples, float pitchShiftRatio);
    
    // PSOLA methods
    void extractGrains(const float* buffer, int numSamples, float pitch);
    void synthesizeGrains(float* outputBuffer, int numSamples, 
                         float targetPitch, float speed, float amount);
    void applyGrain(float* outputBuffer, const GrainData& grain, 
                   int startPosition, float amplitude);
    
    // Utility methods
    float calculateRMS(const float* buffer, int numSamples);
    float calculateCentroid(const std::vector<float>& spectrum);
    void smoothPitch(float newPitch);
    float interpolateValue(float current, float target, float factor);
    
    // Window functions
    void applyHannWindow(float* buffer, int numSamples);
    void applyBlackmanWindow(float* buffer, int numSamples);
    
    // REAL PITCH SHIFTING METHODS
    void applyGranularPitchShift(float* buffer, int numSamples, float pitchRatio, float speed);
    void applyHardPitchQuantization(float* buffer, int numSamples, float pitchRatio, float speed);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchCorrectionEngine)
};
