#include "crepe.h"
#include <cmath>
#include <algorithm>
#include <numeric>

bool CrepeModel::initialized = false;
std::array<float, 360> CrepeModel::noteFrequencies = {};

void CrepeModel::initialize() {
    if (!initialized) {
        generateNoteFrequencies();
        initialized = true;
    }
}

bool CrepeModel::isInitialized() {
    return initialized;
}

void CrepeModel::generateNoteFrequencies() {
    const float A4_FREQ = 440.0f;
    const float A4_MIDI = 69.0f;
    
    for (int i = 0; i < 360; ++i) {
        float midiNote = (float)i;
        noteFrequencies[i] = A4_FREQ * std::pow(2.0f, (midiNote - A4_MIDI) / 12.0f);
    }
}

CrepeModel::PitchResult CrepeModel::estimatePitch(const std::vector<float>& audioBuffer, float sampleRate) {
    if (!initialized) initialize();
    
    if (audioBuffer.size() < 1024) {
        return {0.0f, 0.0f};
    }
    
    // Use YIN algorithm for better pitch detection
    float frequency = yin(audioBuffer, sampleRate);
    float confidence = (frequency > 0.0f) ? 0.8f : 0.0f;
    
    return {frequency, confidence};
}

float CrepeModel::yin(const std::vector<float>& signal, float sampleRate) {
    size_t bufferSize = signal.size();
    size_t halfBufferSize = bufferSize / 2;
    std::vector<float> yinBuffer(halfBufferSize, 0.0f);
    
    // Step 1: Difference function
    for (size_t tau = 1; tau < halfBufferSize; ++tau) {
        for (size_t i = 0; i < halfBufferSize; ++i) {
            float delta = signal[i] - signal[i + tau];
            yinBuffer[tau] += delta * delta;
        }
    }
    
    // Step 2: Cumulative mean normalized difference
    yinBuffer[0] = 1.0f;
    float runningSum = 0.0f;
    for (size_t tau = 1; tau < halfBufferSize; ++tau) {
        runningSum += yinBuffer[tau];
        yinBuffer[tau] *= tau / runningSum;
    }
    
    // Step 3: Absolute threshold
    const float threshold = 0.1f;
    size_t minTau = 2;
    for (size_t tau = minTau; tau < halfBufferSize; ++tau) {
        if (yinBuffer[tau] < threshold) {
            // Step 4: Parabolic interpolation
            float x0 = (tau < 1) ? yinBuffer[tau] : yinBuffer[tau - 1];
            float x2 = (tau + 1 >= halfBufferSize) ? yinBuffer[tau] : yinBuffer[tau + 1];
            
            float a = (x2 + x0 - 2.0f * yinBuffer[tau]) / 2.0f;
            float b = (x2 - x0) / 2.0f;
            
            float betterTau = (a != 0.0f) ? tau - b / (2.0f * a) : tau;
            
            return sampleRate / betterTau;
        }
    }
    
    return 0.0f; // No fundamental found
}

float CrepeModel::autocorrelation(const std::vector<float>& signal, int lag) {
    float sum = 0.0f;
    int count = 0;
    
    for (size_t i = 0; i < signal.size() - lag; ++i) {
        sum += signal[i] * signal[i + lag];
        count++;
    }
    
    return count > 0 ? sum / count : 0.0f;
}