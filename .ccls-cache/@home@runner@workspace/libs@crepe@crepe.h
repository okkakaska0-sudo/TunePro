#pragma once
#include <vector>
#include <array>
#include <memory>

// Forward declaration to avoid TensorFlow Lite dependency in header
namespace tflite {
    class Interpreter;
    class FlatBufferModel;
}

// CREPE AI pitch detection with TensorFlow Lite backend
class CrepeModel {
public:
    struct PitchResult {
        float frequency;
        float confidence;
        bool isValid() const { return frequency > 0.0f && confidence > 0.1f; }
    };
    
    // Main API
    static PitchResult estimatePitch(const std::vector<float>& audioBuffer, float sampleRate);
    static bool initialize();
    static bool isInitialized();
    static void shutdown();
    
    // Advanced configuration
    static void setModelPath(const std::string& path);
    static void setViterbiDecoder(bool enabled);
    static void setCenterFrequency(bool center);
    
private:
    static bool initialized_;
    static std::unique_ptr<tflite::Interpreter> interpreter_;
    static std::unique_ptr<tflite::FlatBufferModel> model_;
    
    // CREPE constants
    static constexpr size_t CREPE_MODEL_CAPACITY = 1024;
    static constexpr size_t CREPE_CENTS_MAPPING_SIZE = 360;
    static constexpr float MIN_FREQUENCY = 50.0f;   // ~G1
    static constexpr float MAX_FREQUENCY = 2000.0f; // ~B6
    
    // Internal processing
    static std::array<float, CREPE_CENTS_MAPPING_SIZE> centsMapping_;
    static void generateCentsMapping();
    static float centsToFrequency(float cents);
    static float frequencyToCents(float frequency);
    
    // TensorFlow Lite integration
    static bool loadModel();
    static std::vector<float> preprocessAudio(const std::vector<float>& audio, float sampleRate);
    static PitchResult postprocessOutput(const float* output, size_t outputSize);
    
    // Fallback algorithms for robustness
    static PitchResult yinPitchDetection(const std::vector<float>& signal, float sampleRate);
    static PitchResult autocorrelationPitch(const std::vector<float>& signal, float sampleRate);
    static float autocorrelation(const std::vector<float>& signal, int lag);
    
    // Utility functions
    static void applyHanningWindow(std::vector<float>& buffer);
    static float calculateRMS(const std::vector<float>& buffer);
    static bool isAudioValid(const std::vector<float>& buffer);
};