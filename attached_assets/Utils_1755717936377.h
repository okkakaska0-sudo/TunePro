#pragma once

#include "JuceHeader.h"
#include "Parameters.h"
#include <vector>
#include <cmath>

class Utils
{
public:
    // Musical note and frequency conversions
    static float frequencyToMidiNote(float frequency);
    static float midiNoteToFrequency(float midiNote);
    
    // Scale quantization
    static float quantizeToScale(float midiNote, Parameters::Key key, Parameters::Scale scale);
    static int findNearestScaleNote(float midiNote, const std::vector<int>& scaleNotes, int keyOffset);
    
    // Audio utility functions
    static float linearToDecibels(float linearValue);
    static float decibelsToLinear(float decibels);
    static float rmsToDecibels(float rmsValue);
    
    // Interpolation functions
    static float linearInterpolation(float x1, float y1, float x2, float y2, float x);
    static float cubicInterpolation(float y0, float y1, float y2, float y3, float x);
    static float hermiteInterpolation(float y0, float y1, float y2, float y3, float x);
    
    // Window type enumeration
    enum class WindowType
    {
        Rectangular,
        Hann,
        Hamming,
        Blackman,
        Kaiser
    };
    
    // Window functions
    static void applyWindow(float* buffer, int numSamples, WindowType windowType);
    
    // FFT utilities
    static void computeFFT(const float* input, std::complex<float>* output, int size);
    static void computeIFFT(const std::complex<float>* input, float* output, int size);
    
    // Pitch analysis utilities
    static float detectPitchZeroCrossing(const float* buffer, int numSamples, float sampleRate);
    static float calculateSpectralCentroid(const std::vector<float>& magnitude, float sampleRate);
    static std::vector<int> findSpectralPeaks(const std::vector<float>& magnitude, float threshold = 0.1f);
    
    // Musical theory utilities
    static String noteNumberToNoteName(int noteNumber);
    static int noteNameToNoteNumber(const String& noteName);
    static float centsToRatio(float cents);
    static float ratioToCents(float ratio);
    
    // Audio processing utilities
    static void normalize(float* buffer, int numSamples, float targetLevel = 1.0f);
    static float calculateCorrelation(const float* buffer1, const float* buffer2, int numSamples);
    static void fadeInOut(float* buffer, int numSamples, int fadeLength);
    
    // Math utilities
    static float fastSin(float x);
    static float fastCos(float x);
    static float fastAtan2(float y, float x);
    static float fastLog2(float x);
    static float fastPow(float base, float exponent);
    
    // Constants
    static constexpr float PI = 3.14159265359f;
    static constexpr float TWO_PI = 2.0f * PI;
    static constexpr float HALF_PI = PI * 0.5f;
    static constexpr float INV_PI = 1.0f / PI;
    static constexpr float INV_TWO_PI = 1.0f / TWO_PI;
    
    // Musical constants
    static constexpr float CONCERT_A_FREQ = 440.0f;
    static constexpr float MIDI_A4 = 69.0f;
    static constexpr float SEMITONE_RATIO = 1.0594630943593f; // 2^(1/12)
    static constexpr float CENTS_PER_SEMITONE = 100.0f;
    
private:
    Utils() = delete; // Static class only
    
    // Fast lookup tables (could be initialized statically)
    static std::vector<float> sinLookupTable;
    static std::vector<float> cosLookupTable;
    static constexpr int LOOKUP_TABLE_SIZE = 8192;
    
    // Internal utility functions
    static void initializeLookupTables();
    static float lookupSin(float x);
    static float lookupCos(float x);
    
    JUCE_DECLARE_NON_COPYABLE(Utils)
};
