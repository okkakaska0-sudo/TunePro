#pragma once

#include "JuceHeader.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>

#ifdef USE_ONNX
  #ifdef __has_include
    #if __has_include(<onnxruntime_cxx_api.h>)
      #include <onnxruntime_cxx_api.h>
      #define ONNX_AVAILABLE 1
    #else
      #define ONNX_AVAILABLE 0
    #endif
  #else
    #define ONNX_AVAILABLE 0
  #endif
#else
  #define ONNX_AVAILABLE 0
#endif

#ifdef USE_EIGEN
#include <Eigen/Dense>
#include <Eigen/Core>
#endif

#ifdef USE_FFTW
#include <fftw3.h>
#endif

// Forward declarations for AI model interfaces
namespace AIModels {
    class CrepeModel;
    class DDSPModel;
}

class AIModelLoader
{
public:
    AIModelLoader();
    ~AIModelLoader();

    // Model loading and management
    bool loadCrepeModel(const File& modelFile);
    bool loadDDSPModel(const File& modelFile);
    bool areModelsLoaded() const;
    
    // Model information
    struct ModelInfo
    {
        String name;
        String version;
        String description;
        float sampleRate;
        int inputSize;
        int outputSize;
        bool isLoaded;
        
        ModelInfo() : sampleRate(44100.0f), inputSize(0), outputSize(0), isLoaded(false) {}
    };
    
    ModelInfo getCrepeModelInfo() const { return crepeInfo; }
    ModelInfo getDDSPModelInfo() const { return ddspInfo; }
    
    // AI Processing methods
    struct PitchPrediction
    {
        float frequency;
        float confidence;
        float voicing;
        std::vector<float> harmonics;
        
        PitchPrediction() : frequency(0.0f), confidence(0.0f), voicing(0.0f) {}
    };
    
    // CREPE pitch detection
    PitchPrediction predictPitch(const float* audioBuffer, int numSamples, float sampleRate);
    std::vector<PitchPrediction> predictPitchBatch(const std::vector<std::vector<float>>& audioBuffers, float sampleRate);
    
    // DDSP synthesis
    struct SynthesisParams
    {
        float fundamentalFreq;
        std::vector<float> harmonicAmplitudes;
        std::vector<float> noiseLevel;
        float loudness;
        
        SynthesisParams() : fundamentalFreq(0.0f), loudness(0.0f) {}
    };
    
    std::vector<float> synthesizeAudio(const SynthesisParams& params, int numSamples, float sampleRate);
    bool processWithDDSP(const float* inputBuffer, float* outputBuffer, int numSamples, 
                        const SynthesisParams& targetParams);
    
    // Model management
    void unloadModels();
    bool reloadModels();
    void setModelDirectory(const File& directory);
    File getModelDirectory() const { return modelDirectory; }
    
    // Performance settings
    enum class ProcessingQuality
    {
        Draft,      // Fastest, lower quality
        Standard,   // Balanced
        High,       // Slower, higher quality
        Ultra       // Slowest, best quality
    };
    
    void setProcessingQuality(ProcessingQuality quality);
    ProcessingQuality getProcessingQuality() const { return currentQuality; }
    
    // Threading and performance
    void setUseMultiThreading(bool useThreads);
    bool isUsingMultiThreading() const { return useMultiThreading; }
    void setMaxThreads(int maxThreads);
    
    // Error handling
    struct AIError
    {
        enum Type
        {
            None,
            ModelNotFound,
            ModelLoadFailed,
            IncompatibleModel,
            ProcessingError,
            OutOfMemory
        };
        
        Type type;
        String message;
        
        AIError() : type(None) {}
        AIError(Type t, const String& msg) : type(t), message(msg) {}
        bool operator==(const AIError& other) const { return type == other.type; }
    };
    
    AIError getLastError() const { return lastError; }
    bool hasError() const { return lastError.type != AIError::None; }
    void clearError() { lastError = AIError(); }
    
    // Callbacks
    std::function<void(const String&)> onModelLoaded;
    std::function<void(const AIError&)> onError;
    std::function<void(float)> onProgress;

private:
    // Model instances (using PIMPL pattern to hide implementation)
    struct Impl;
    std::unique_ptr<Impl> pImpl;
    
    // Model information
    ModelInfo crepeInfo;
    ModelInfo ddspInfo;
    
    // Configuration
    File modelDirectory;
    ProcessingQuality currentQuality;
    bool useMultiThreading;
    int maxThreads;
    
    // Error tracking
    AIError lastError;
    
    // Internal methods
    bool initializeCrepeModel(const File& modelFile);
    bool initializeDDSPModel(const File& modelFile);
    void setupDefaultModelDirectory();
    bool validateModelFile(const File& file, const String& expectedType);
    
    // Processing helpers
    std::vector<float> preprocessAudioForCrepe(const float* input, int numSamples, float targetSampleRate);
    SynthesisParams extractSynthesisParams(const float* audioBuffer, int numSamples, float sampleRate);
    void postprocessDDSPOutput(float* output, int numSamples, float gainAdjustment);
    
    // Thread management
    ThreadPool threadPool;
    CriticalSection modelLock;
    
    // Performance monitoring
    void updatePerformanceMetrics();
    float lastProcessingTime;
    float averageProcessingTime;
    int processedFrames;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AIModelLoader)
};