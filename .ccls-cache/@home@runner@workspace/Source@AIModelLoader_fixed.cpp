#include "AIModelLoader.h"
#include "Utils.h"
#include <cstring>

AIModelLoader::AIModelLoader()
{
    pitchHistory.resize(10, 0.0f);
    
    // Initialize FFT for spectral analysis
    fft = std::make_unique<juce::dsp::FFT>(fftOrder);
    frequencyData.allocate(fftSize * 2, true);
    
    // Initialize DDSP synthesizer
    synthesizer = std::make_unique<DDSPSynthesizer>();
    synthesizer->reverbBuffer.setSize(1, 4410); // 100ms reverb buffer at 44.1kHz
    synthesizer->reverbBuffer.clear();
    
    prepareToPlay(44100.0, 512);
}

AIModelLoader::~AIModelLoader()
{
    unloadModels();
}

bool AIModelLoader::loadModels()
{
    // For MVP, we simulate model loading
    // In a full implementation, this would load actual CREPE and DDSP models
    
    lastProcessTime = juce::Time::getCurrentTime();
    
    // Simulate model loading time
    juce::Thread::sleep(100);
    
    modelsLoaded = true;
    
    juce::Logger::writeToLog("AI Models loaded successfully (simulated)");
    return true;
}

void AIModelLoader::unloadModels()
{
    modelsLoaded = false;
    
    // Reset synthesis state
    if (synthesizer)
    {
        std::fill(synthesizer->harmonicPhases.begin(), synthesizer->harmonicPhases.end(), 0.0f);
        std::fill(synthesizer->harmonicAmps.begin(), synthesizer->harmonicAmps.end(), 0.0f);
        synthesizer->reverbBuffer.clear();
        synthesizer->reverbPosition = 0;
    }
    
    juce::Logger::writeToLog("AI Models unloaded");
}

AIModelLoader::PitchPrediction AIModelLoader::predictPitch(const float* audio, int numSamples, double sampleRate)
{
    PitchPrediction prediction;
    
    if (!modelsLoaded || numSamples == 0)
        return prediction;
    
    auto startTime = juce::Time::getMillisecondCounter();
    
    // Simulate CREPE-style pitch detection
    float rawPitch = detectPitchCREPESimulation(audio, numSamples);
    
    if (rawPitch > 0.0f)
    {
        // Apply sophisticated pitch tracking and smoothing
        prediction.frequency = smoothPitchEstimate(rawPitch);
        
        // Analyze full pitch features
        prediction = analyzePitchFeatures(audio, numSamples);
        
        // Calculate confidence based on signal characteristics
        float rms = 0.0f;
        for (int i = 0; i < numSamples; ++i)
        {
            rms += audio[i] * audio[i];
        }
        rms = std::sqrt(rms / numSamples);
        
        prediction.confidence = juce::jlimit(0.0f, 1.0f, rms * 10.0f); // Scale RMS to confidence
        
        // Extract harmonics from spectrum
        std::vector<float> spectrum;
        performSpectralAnalysis(audio, numSamples, spectrum);
        extractHarmonics(spectrum, prediction.frequency, prediction.harmonics);
        
        // Estimate voicing (harmonic vs noise content)
        float harmonicEnergy = 0.0f;
        for (float harmonic : prediction.harmonics)
        {
            harmonicEnergy += harmonic;
        }
        prediction.voicing = juce::jlimit(0.0f, 1.0f, harmonicEnergy * 2.0f);
    }
    
    // Update performance metrics
    processingTimeMs = juce::Time::getMillisecondCounter() - startTime;
    updatePerformanceMetrics();
    
    return prediction;
}

bool AIModelLoader::processWithDDSP(const float* input, float* output, int numSamples, 
                                    const SynthesisParams& params)
{
    if (!modelsLoaded || !synthesizer)
        return false;
    
    auto startTime = juce::Time::getMillisecondCounter();
    
    // Copy input to output as base
    std::memcpy(output, input, numSamples * sizeof(float));
    
    // Prepare temporary buffers
    processBuffer.setSize(1, numSamples);
    processBuffer.clear();
    
    auto* processData = processBuffer.getWritePointer(0);
    
    // Synthesize harmonics
    synthesizeHarmonics(processData, numSamples, params);
    
    // Add noise component
    if (params.noisiness > 0.0f)
    {
        juce::AudioBuffer<float> noiseBuffer(1, numSamples);
        auto* noiseData = noiseBuffer.getWritePointer(0);
        synthesizeNoise(noiseData, numSamples, params.noisiness);
        
        // Mix noise with harmonics
        for (int i = 0; i < numSamples; ++i)
        {
            processData[i] += noiseData[i] * params.noisiness;
        }
    }
    
    // Apply formant filtering to maintain vocal character
    applyFormantFiltering(processData, numSamples, params.fundamentalFreq);
    
    // Apply loudness control
    float gainMultiplier = params.loudness * 2.0f; // Scale to appropriate range
    for (int i = 0; i < numSamples; ++i)
    {
        processData[i] *= gainMultiplier;
    }
    
    // Mix processed signal with original
    for (int i = 0; i < numSamples; ++i)
    {
        output[i] = output[i] * 0.3f + processData[i] * 0.7f; // Favor synthesis
    }
    
    // Update performance metrics
    processingTimeMs = juce::Time::getMillisecondCounter() - startTime;
    updatePerformanceMetrics();
    
    return true;
}

float AIModelLoader::detectPitchCREPESimulation(const float* audio, int numSamples)
{
    if (numSamples < 64)
        return 0.0f;
    
    // Simple autocorrelation-based pitch detection to simulate CREPE
    std::vector<float> autocorr(numSamples / 2, 0.0f);
    
    // Calculate autocorrelation function
    for (int lag = 1; lag < numSamples / 2; ++lag)
    {
        for (int i = 0; i < numSamples - lag; ++i)
        {
            autocorr[lag] += audio[i] * audio[i + lag];
        }
        autocorr[lag] /= (numSamples - lag); // Normalize
    }
    
    // Find the lag with maximum correlation
    int bestLag = 1;
    float maxCorr = autocorr[1];
    
    for (int lag = 2; lag < numSamples / 2; ++lag)
    {
        if (autocorr[lag] > maxCorr && lag > 16) // Avoid harmonics
        {
            maxCorr = autocorr[lag];
            bestLag = lag;
        }
    }
    
    // Convert lag to frequency
    if (bestLag > 0 && maxCorr > 0.3f) // Confidence threshold
    {
        return 44100.0f / bestLag; // Assume 44.1kHz sample rate
    }
    
    return 0.0f; // No pitch detected
}

AIModelLoader::PitchPrediction AIModelLoader::analyzePitchFeatures(const float* audio, int numSamples)
{
    PitchPrediction prediction;
    
    // Detect fundamental frequency using YIN algorithm simulation
    float fundamental = detectPitchCREPESimulation(audio, numSamples);
    prediction.frequency = fundamental;
    
    if (fundamental > 0.0f)
    {
        // Analyze harmonics up to Nyquist frequency
        std::vector<float> spectrum;
        performSpectralAnalysis(audio, numSamples, spectrum);
        
        // Extract harmonics at multiples of fundamental
        prediction.harmonics.resize(16, 0.0f);
        for (int h = 1; h <= 16; ++h)
        {
            float harmonicFreq = fundamental * h;
            if (harmonicFreq < 22050.0f) // Below Nyquist
            {
                int binIndex = static_cast<int>((harmonicFreq / 22050.0f) * spectrum.size());
                if (binIndex < spectrum.size())
                {
                    prediction.harmonics[h-1] = spectrum[binIndex];
                }
            }
        }
        
        // Calculate voicing based on harmonic content
        float totalEnergy = 0.0f;
        float harmonicEnergy = 0.0f;
        
        for (float mag : spectrum)
        {
            totalEnergy += mag;
        }
        
        for (float harmonic : prediction.harmonics)
        {
            harmonicEnergy += harmonic;
        }
        
        prediction.voicing = (totalEnergy > 0.0f) ? (harmonicEnergy / totalEnergy) : 0.0f;
        prediction.confidence = juce::jlimit(0.0f, 1.0f, prediction.voicing * 2.0f);
    }
    
    return prediction;
}

void AIModelLoader::performSpectralAnalysis(const float* audio, int numSamples, std::vector<float>& spectrum)
{
    // Zero-pad to FFT size
    int paddedSize = fftSize;
    std::vector<float> paddedAudio(paddedSize, 0.0f);
    
    int copySize = std::min(numSamples, paddedSize);
    std::memcpy(paddedAudio.data(), audio, copySize * sizeof(float));
    
    // Apply window function (Hanning)
    for (int i = 0; i < copySize; ++i)
    {
        float window = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (copySize - 1)));
        paddedAudio[i] *= window;
    }
    
    // Convert to complex numbers for FFT
    for (int i = 0; i < paddedSize; ++i)
    {
        frequencyData[i].real(paddedAudio[i]);
        frequencyData[i].imag(0.0f);
    }
    
    // Perform FFT
    fft->performFrequencyOnlyForwardTransform(frequencyData.getData(), true);
    
    // Calculate magnitude spectrum
    spectrum.resize(paddedSize / 2);
    for (int i = 0; i < paddedSize / 2; ++i)
    {
        float real = frequencyData[i].real();
        float imag = frequencyData[i].imag();
        spectrum[i] = std::sqrt(real * real + imag * imag);
    }
}

void AIModelLoader::extractHarmonics(const std::vector<float>& spectrum, float fundamental, std::vector<float>& harmonics)
{
    harmonics.resize(16, 0.0f);
    
    if (fundamental <= 0.0f || spectrum.empty())
        return;
    
    float sampleRate = 44100.0f; // Assume fixed sample rate
    float binWidth = sampleRate / (2.0f * spectrum.size());
    
    for (int h = 1; h <= 16; ++h)
    {
        float harmonicFreq = fundamental * h;
        if (harmonicFreq >= sampleRate / 2.0f) // Above Nyquist
            break;
            
        int binIndex = static_cast<int>(harmonicFreq / binWidth);
        if (binIndex < spectrum.size())
        {
            harmonics[h-1] = spectrum[binIndex];
        }
    }
}

void AIModelLoader::synthesizeHarmonics(float* output, int numSamples, const SynthesisParams& params)
{
    if (!synthesizer || params.fundamentalFreq <= 0.0f)
        return;
    
    // Ensure we have enough harmonics
    if (synthesizer->harmonicPhases.size() < 16)
    {
        synthesizer->harmonicPhases.resize(16, 0.0f);
        synthesizer->harmonicFreqs.resize(16, 0.0f);
        synthesizer->harmonicAmps.resize(16, 0.0f);
    }
    
    // Update harmonic frequencies and amplitudes
    for (int h = 0; h < 16; ++h)
    {
        synthesizer->harmonicFreqs[h] = params.fundamentalFreq * (h + 1);
        synthesizer->harmonicAmps[h] = (h < params.harmonicAmplitudes.size()) ? 
            params.harmonicAmplitudes[h] : 0.0f;
    }
    
    float sampleRate = 44100.0f;
    
    // Generate harmonic content
    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.0f;
        
        for (int h = 0; h < 16; ++h)
        {
            if (synthesizer->harmonicAmps[h] > 0.001f && 
                synthesizer->harmonicFreqs[h] < sampleRate / 2.0f)
            {
                // Generate sine wave for this harmonic
                sample += synthesizer->harmonicAmps[h] * 
                         std::sin(synthesizer->harmonicPhases[h]);
                
                // Update phase
                synthesizer->harmonicPhases[h] += 2.0f * M_PI * 
                                                 synthesizer->harmonicFreqs[h] / sampleRate;
                
                // Keep phase in range
                if (synthesizer->harmonicPhases[h] > 2.0f * M_PI)
                    synthesizer->harmonicPhases[h] -= 2.0f * M_PI;
            }
        }
        
        output[i] = sample * params.loudness;
    }
}

void AIModelLoader::synthesizeNoise(float* output, int numSamples, float noisiness)
{
    if (!synthesizer)
        return;
    
    // Generate colored noise
    for (int i = 0; i < numSamples; ++i)
    {
        // Generate white noise
        float noise = synthesizer->noiseGenerator.nextFloat() * 2.0f - 1.0f;
        
        // Simple low-pass filtering for more natural noise
        if (i > 0)
            noise = noise * 0.7f + output[i - 1] * 0.3f;
        
        output[i] = noise * noisiness * 0.05f; // Scale for appropriate level
    }
}

void AIModelLoader::applyFormantFiltering(float* audio, int numSamples, float fundamentalFreq)
{
    // Simplified formant filtering to preserve vocal character
    // Real implementation would use proper formant filters
    
    if (fundamentalFreq <= 0.0f)
        return;
    
    // Apply gentle high-frequency emphasis to maintain clarity
    float prev = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        float filtered = audio[i] * 0.9f + (audio[i] - prev) * 0.1f;
        prev = audio[i];
        audio[i] = filtered;
    }
}

void AIModelLoader::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    processingBlockSize = samplesPerBlock;
    
    // Prepare buffers
    processBuffer.setSize(1, samplesPerBlock);
    analysisBuffer.setSize(1, samplesPerBlock * 2);
    windowBuffer.resize(samplesPerBlock);
    
    // Reset synthesis state
    if (synthesizer)
    {
        std::fill(synthesizer->harmonicPhases.begin(), synthesizer->harmonicPhases.end(), 0.0f);
        synthesizer->reverbBuffer.setSize(1, static_cast<int>(sampleRate * 0.1)); // 100ms
        synthesizer->reverbBuffer.clear();
        synthesizer->reverbPosition = 0;
    }
}

void AIModelLoader::updatePerformanceMetrics()
{
    // Simple CPU usage estimation based on processing time
    float processingRatio = static_cast<float>(processingTimeMs) / 30.0f; // Assuming 30ms budget
    cpuUsage = cpuUsage * 0.9f + processingRatio * 0.1f; // Smooth the measurement
}

float AIModelLoader::smoothPitchEstimate(float newPitch)
{
    if (lastPitchEstimate == 0.0f)
    {
        lastPitchEstimate = newPitch;
        return newPitch;
    }
    
    // Apply exponential smoothing
    float smoothed = lastPitchEstimate * (1.0f - pitchSmoothing) + newPitch * pitchSmoothing;
    
    // Add pitch to history
    pitchHistory.push_back(smoothed);
    if (pitchHistory.size() > 10)
    {
        pitchHistory.erase(pitchHistory.begin());
    }
    
    lastPitchEstimate = smoothed;
    return smoothed;
}