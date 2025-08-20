#pragma once
#include <vector>
#include <array>

// CREPE AI pitch detection implementation
class CrepeModel {
public:
    struct PitchResult {
        float frequency;
        float confidence;
    };
    
    static PitchResult estimatePitch(const std::vector<float>& audioBuffer, float sampleRate);
    static void initialize();
    static bool isInitialized();
    
private:
    static bool initialized;
    static std::array<float, 360> noteFrequencies;
    static void generateNoteFrequencies();
    static float autocorrelation(const std::vector<float>& signal, int lag);
    static float yin(const std::vector<float>& signal, float sampleRate);
};