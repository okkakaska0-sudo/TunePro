#include "ModeSelector.h"

ModeSelector::ModeSelector()
    : currentMode(Parameters::Mode::Classic),
      currentQuality(QualityLevel::High),
      isInitialized(false)
{
    initializeModeConfigs();
    isInitialized = true;
}

ModeSelector::~ModeSelector()
{
}

void ModeSelector::setCurrentMode(Parameters::Mode mode)
{
    if (mode != currentMode && canSwitchToMode(mode))
    {
        Parameters::Mode oldMode = currentMode;
        currentMode = mode;
        lastSwitchTime = Time::getCurrentTime();
        
        if (onModeChanged)
            onModeChanged(oldMode, currentMode);
    }
}

const ModeSelector::ModeConfig& ModeSelector::getCurrentModeConfig() const
{
    return getModeConfig(currentMode);
}

const ModeSelector::ModeConfig& ModeSelector::getModeConfig(Parameters::Mode mode) const
{
    int index = static_cast<int>(mode);
    if (index >= 0 && index < static_cast<int>(modeConfigs.size()))
    {
        return modeConfigs[index];
    }
    return modeConfigs[0]; // Default to Classic mode
}

StringArray ModeSelector::getModeNames() const
{
    StringArray names;
    for (const auto& config : modeConfigs)
    {
        names.add(config.name);
    }
    return names;
}

ModeSelector::ProcessingParams ModeSelector::getProcessingParams() const
{
    return getProcessingParams(currentMode);
}

ModeSelector::ProcessingParams ModeSelector::getProcessingParams(Parameters::Mode mode) const
{
    const auto& config = getModeConfig(mode);
    return calculateProcessingParams(config);
}

ModeSelector::PitchAlgorithm ModeSelector::getPitchAlgorithm() const
{
    return getPitchAlgorithm(currentMode);
}

ModeSelector::PitchAlgorithm ModeSelector::getPitchAlgorithm(Parameters::Mode mode) const
{
    switch (mode)
    {
        case Parameters::Mode::Classic:
            return PitchAlgorithm::Autocorrelation;
        case Parameters::Mode::Hard:
            return PitchAlgorithm::YIN;
        case Parameters::Mode::AI:
            return PitchAlgorithm::Combined;
        default:
            return PitchAlgorithm::Autocorrelation;
    }
}

ModeSelector::QualityLevel ModeSelector::getQualityLevel() const
{
    return currentQuality;
}

void ModeSelector::setQualityLevel(QualityLevel level)
{
    currentQuality = level;
}

void ModeSelector::switchToMode(Parameters::Mode newMode)
{
    if (!validateModeSwitch(newMode))
    {
        if (onModeError)
            onModeError("Cannot switch to mode: " + getModeConfig(newMode).name);
        return;
    }
    
    setCurrentMode(newMode);
}

bool ModeSelector::canSwitchToMode(Parameters::Mode mode) const
{
    // Check if enough time has passed since last switch to prevent rapid switching
    if (isInitialized && (Time::getCurrentTime() - lastSwitchTime).inMilliseconds() < 100)
    {
        return false;
    }
    
    // Check if mode is valid
    int index = static_cast<int>(mode);
    if (index < 0 || index >= static_cast<int>(modeConfigs.size()))
    {
        return false;
    }
    
    return true;
}

ModeSelector::PerformanceMetrics ModeSelector::getPerformanceMetrics() const
{
    return metrics;
}

void ModeSelector::updatePerformanceMetrics(float cpu, float latency, float accuracy)
{
    metrics.cpuUsage = cpu;
    metrics.latency = latency;
    metrics.accuracy = accuracy;
    
    // Determine if real-time capable based on current metrics
    const auto& config = getCurrentModeConfig();
    metrics.realTimeCapable = (latency <= config.latencyMs * 1.5f) && (cpu <= 80.0f);
}

void ModeSelector::initializeModeConfigs()
{
    // Classic Mode - Traditional auto-tune with smooth correction
    modeConfigs[0] = ModeConfig(
        "Classic",
        "Traditional smooth pitch correction with natural sound",
        50.0f,  // Default speed
        50.0f,  // Default amount
        false,  // Formant preservation (basic)
        false,  // Advanced pitch detection
        false,  // Rubber Band
        8.0f    // Latency in ms
    );
    
    // Hard Mode - Aggressive T-Pain style correction
    modeConfigs[1] = ModeConfig(
        "Hard",
        "Aggressive pitch correction with immediate snapping",
        90.0f,  // Default speed (fast)
        85.0f,  // Default amount (strong)
        false,  // Formant preservation (not needed for effect)
        true,   // Advanced pitch detection (YIN for accuracy)
        false,  // Rubber Band (not needed for hard correction)
        5.0f    // Latency in ms (very responsive)
    );
    
    // AI Mode - Advanced processing with formant preservation
    modeConfigs[2] = ModeConfig(
        "AI",
        "AI-powered natural correction with formant preservation",
        25.0f,  // Default speed (slow and natural)
        60.0f,  // Default amount (moderate)
        true,   // Formant preservation (essential for natural sound)
        true,   // Advanced pitch detection (combined algorithms)
        true,   // Rubber Band (high quality processing)
        15.0f   // Latency in ms (higher due to advanced processing)
    );
}

ModeSelector::ProcessingParams ModeSelector::calculateProcessingParams(const ModeConfig& config) const
{
    ProcessingParams params;
    
    // Base parameters from mode config
    params.speedMultiplier = config.defaultSpeed / 50.0f; // Normalize to 50% base
    params.amountMultiplier = config.defaultAmount / 50.0f;
    
    // Adjust based on current mode
    switch (currentMode)
    {
        case Parameters::Mode::Classic:
            params.smoothingFactor = 0.8f;
            params.windowSize = 2048;
            params.hopSize = 512;
            params.enableFormantCorrection = false;
            params.enableHarmonicAnalysis = false;
            break;
            
        case Parameters::Mode::Hard:
            params.smoothingFactor = 0.2f; // Less smoothing for immediate response
            params.windowSize = 1024;      // Smaller window for lower latency
            params.hopSize = 256;          // Smaller hop for faster response
            params.enableFormantCorrection = false;
            params.enableHarmonicAnalysis = true; // For better pitch detection
            break;
            
        case Parameters::Mode::AI:
            params.smoothingFactor = 0.9f; // More smoothing for natural sound
            params.windowSize = 4096;      // Larger window for better analysis
            params.hopSize = 1024;         // Larger hop acceptable for quality
            params.enableFormantCorrection = true;
            params.enableHarmonicAnalysis = true;
            break;
    }
    
    // Adjust based on quality level
    switch (currentQuality)
    {
        case QualityLevel::Draft:
            params.windowSize /= 2;
            params.hopSize /= 2;
            params.enableFormantCorrection = false;
            break;
            
        case QualityLevel::Good:
            // Use base settings
            break;
            
        case QualityLevel::High:
            // Use base settings with enhancements
            if (currentMode == Parameters::Mode::AI)
            {
                params.smoothingFactor *= 1.1f;
            }
            break;
            
        case QualityLevel::Ultra:
            params.windowSize *= 2;
            params.enableFormantCorrection = true;
            params.enableHarmonicAnalysis = true;
            params.smoothingFactor = jmin(0.95f, params.smoothingFactor * 1.2f);
            break;
    }
    
    return params;
}

bool ModeSelector::validateModeSwitch(Parameters::Mode newMode) const
{
    // Basic validation
    if (!canSwitchToMode(newMode))
        return false;
    
    // Check if we have the required resources for the new mode
    const auto& config = getModeConfig(newMode);
    
    // AI mode requires more CPU - check if system can handle it
    if (newMode == Parameters::Mode::AI && metrics.cpuUsage > 70.0f)
    {
        return false; // System might be too loaded for AI mode
    }
    
    // Hard mode requires low latency - check if achievable
    if (newMode == Parameters::Mode::Hard && metrics.latency > 20.0f)
    {
        return false; // Current latency too high for hard mode
    }
    
    return true;
}
