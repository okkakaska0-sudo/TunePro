#pragma once

#include "JuceHeader.h"
#include "Parameters.h"

class ModeSelector
{
public:
    ModeSelector();
    ~ModeSelector();

    // Mode management
    void setCurrentMode(Parameters::Mode mode);
    Parameters::Mode getCurrentMode() const { return currentMode; }
    
    // Mode configuration
    struct ModeConfig
    {
        String name;
        String description;
        float defaultSpeed;
        float defaultAmount;
        bool useFormantPreservation;
        bool useAdvancedPitchDetection;
        bool useRubberBand;
        float latencyMs;
        
        // Default constructor needed for std::array
        ModeConfig() : name(""), description(""), defaultSpeed(50.0f), defaultAmount(50.0f),
                      useFormantPreservation(false), useAdvancedPitchDetection(false),
                      useRubberBand(false), latencyMs(10.0f) {}
        
        ModeConfig(const String& modeName, const String& desc, 
                  float speed, float amount, bool formants = false, 
                  bool advanced = false, bool rubberband = false, float latency = 10.0f)
            : name(modeName), description(desc), defaultSpeed(speed), defaultAmount(amount),
              useFormantPreservation(formants), useAdvancedPitchDetection(advanced),
              useRubberBand(rubberband), latencyMs(latency) {}
    };
    
    // Mode information
    const ModeConfig& getCurrentModeConfig() const;
    const ModeConfig& getModeConfig(Parameters::Mode mode) const;
    StringArray getModeNames() const;
    
    // Processing parameters based on mode
    struct ProcessingParams
    {
        float speedMultiplier;
        float amountMultiplier;
        float smoothingFactor;
        int windowSize;
        int hopSize;
        bool enableFormantCorrection;
        bool enableHarmonicAnalysis;
        
        ProcessingParams() : speedMultiplier(1.0f), amountMultiplier(1.0f), 
                           smoothingFactor(0.8f), windowSize(2048), hopSize(512),
                           enableFormantCorrection(false), enableHarmonicAnalysis(false) {}
    };
    
    ProcessingParams getProcessingParams() const;
    ProcessingParams getProcessingParams(Parameters::Mode mode) const;
    
    // Algorithm selection
    enum class PitchAlgorithm
    {
        Autocorrelation,
        YIN,
        Spectral,
        Harmonic,
        Combined
    };
    
    PitchAlgorithm getPitchAlgorithm() const;
    PitchAlgorithm getPitchAlgorithm(Parameters::Mode mode) const;
    
    // Quality settings
    enum class QualityLevel
    {
        Draft,
        Good,
        High,
        Ultra
    };
    
    QualityLevel getQualityLevel() const;
    void setQualityLevel(QualityLevel level);
    
    // Mode switching
    void switchToMode(Parameters::Mode newMode);
    bool canSwitchToMode(Parameters::Mode mode) const;
    
    // Performance metrics
    struct PerformanceMetrics
    {
        float cpuUsage;
        float latency;
        float accuracy;
        bool realTimeCapable;
        
        PerformanceMetrics() : cpuUsage(0.0f), latency(0.0f), accuracy(0.0f), realTimeCapable(true) {}
    };
    
    PerformanceMetrics getPerformanceMetrics() const;
    void updatePerformanceMetrics(float cpu, float latency, float accuracy);
    
    // Callbacks
    std::function<void(Parameters::Mode, Parameters::Mode)> onModeChanged;
    std::function<void(const String&)> onModeError;

private:
    Parameters::Mode currentMode;
    QualityLevel currentQuality;
    PerformanceMetrics metrics;
    
    // Mode configurations
    std::array<ModeConfig, 3> modeConfigs;
    
    // Performance tracking
    Time lastSwitchTime;
    bool isInitialized;
    
    // Helper methods
    void initializeModeConfigs();
    ProcessingParams calculateProcessingParams(const ModeConfig& config) const;
    bool validateModeSwitch(Parameters::Mode newMode) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeSelector)
};
