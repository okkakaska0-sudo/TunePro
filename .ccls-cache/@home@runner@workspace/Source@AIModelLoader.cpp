#include "AIModelLoader.h"
#include "Utils.h"

AIModelLoader::AIModelLoader()
{
    pitchHistory.resize(10, 0.0f);
    
    // Initialize FFT for spectral analysis
    fft = std::make_unique<dsp::FFT>(fftOrder);
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
    
    lastProcessTime = Time::getCurrentTime();
    
    // Simulate model loading time
    Thread::sleep(100);
    
    modelsLoaded = true;
    
    DBG("AI Models loaded successfully (simulated)");
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
    
    DBG("AI Models unloaded");
}

AIModelLoader::PitchPrediction AIModelLoader::predictPitch(const float* audio, int numSamples, double sampleRate)
{
    PitchPrediction prediction;
    
    if (!modelsLoaded || numSamples == 0)
        return prediction;
    
    auto startTime = Time::getMillisecondCounter();
    
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
        
        prediction.confidence = jlimit(0.0f, 1.0f, rms * 10.0f); // Scale RMS to confidence
        
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
        prediction.voicing = jlimit(0.0f, 1.0f, harmonicEnergy * 2.0f);
    }
    
    // Update performance metrics
    processingTimeMs = Time::getMillisecondCounter() - startTime;
    updatePerformanceMetrics();
    
    return prediction;
}

bool AIModelLoader::processWithDDSP(const float* input, float* output, int numSamples, 
                                    const SynthesisParams& params)
{
    if (!modelsLoaded || !synthesizer)
        return false;
    
    auto startTime = Time::getMillisecondCounter();
    
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
        AudioBuffer<float> noiseBuffer(1, numSamples);
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
    float loudnessScale = params.loudness * 0.5f; // Prevent over-amplification
    for (int i = 0; i < numSamples; ++i)
    {
        processData[i] *= loudnessScale;
    }
    
    // Blend with original signal (50/50 mix for natural sound)
    for (int i = 0; i < numSamples; ++i)
    {
        output[i] = output[i] * 0.5f + processData[i] * 0.5f;
    }
    
    // Update performance metrics
    processingTimeMs = Time::getMillisecondCounter() - startTime;
    updatePerformanceMetrics();
    
    return true;
}

float AIModelLoader::detectPitchCREPESimulation(const float* audio, int numSamples)
{
    // Simulate advanced pitch detection similar to CREPE
    // This uses multiple analysis methods and combines them
    
    // Method 1: Enhanced autocorrelation
    float autocorrPitch = Utils::detectPitchZeroCrossing(audio, numSamples, currentSampleRate);
    
    // Method 2: Spectral analysis
    std::vector<float> spectrum;
    performSpectralAnalysis(audio, numSamples, spectrum);
    
    float spectralPitch = 0.0f;
    if (!spectrum.empty())
    {
        // Find spectral peaks
        auto peaks = Utils::findSpectralPeaks(spectrum, 0.1f);
        
        if (!peaks.empty())
        {
            // Use the first significant peak as fundamental
            int peakBin = peaks[0];
            spectralPitch = (peakBin * currentSampleRate) / fftSize;
        }
    }
    
    // Method 3: Cepstral analysis (simplified)
    float cepstralPitch = 0.0f;
    if (!spectrum.empty())
    {
        // Take log of spectrum
        std::vector<float> logSpectrum(spectrum.size());
        for (size_t i = 0; i < spectrum.size(); ++i)
        {
            logSpectrum[i] = std::log(spectrum[i] + 1e-10f);
        }
        
        // Find periodicity in log spectrum (cepstral analysis)
        // This is a simplified version - real cepstral analysis would use IFFT
        float maxCorrelation = 0.0f;
        int bestQuefrency = 0;
        
        for (int q = 20; q < logSpectrum.size() / 4; ++q) // Search reasonable quefrency range
        {
            float correlation = 0.0f;
            int samples = logSpectrum.size() - q;
            
            for (int i = 0; i < samples; ++i)
            {
                correlation += logSpectrum[i] * logSpectrum[i + q];
            }
            
            if (correlation > maxCorrelation)
            {
                maxCorrelation = correlation;
                bestQuefrency = q;
            }
        }
        
        if (bestQuefrency > 0)
        {
            cepstralPitch = currentSampleRate / (bestQuefrency * 2.0f);
        }
    }
    
    // Combine methods with weighted average
    float finalPitch = 0.0f;
    float totalWeight = 0.0f;
    
    if (autocorrPitch > 20.0f && autocorrPitch < 5000.0f)
    {
        finalPitch += autocorrPitch * 0.4f;
        totalWeight += 0.4f;
    }
    
    if (spectralPitch > 20.0f && spectralPitch < 5000.0f)
    {
        finalPitch += spectralPitch * 0.4f;
        totalWeight += 0.4f;
    }
    
    if (cepstralPitch > 20.0f && cepstralPitch < 5000.0f)
    {
        finalPitch += cepstralPitch * 0.2f;
        totalWeight += 0.2f;
    }
    
    return totalWeight > 0.0f ? finalPitch / totalWeight : 0.0f;
}

AIModelLoader::PitchPrediction AIModelLoader::analyzePitchFeatures(const float* audio, int numSamples)
{
    PitchPrediction prediction;
    
    // Advanced pitch analysis with multiple features
    float pitch = detectPitchCREPESimulation(audio, numSamples);
    prediction.frequency = pitch;
    
    if (pitch > 0.0f)
    {
        // Analyze pitch stability over time
        float pitchVariance = 0.0f;
        if (pitchHistory.size() > 1)
        {
            float meanPitch = 0.0f;
            for (float p : pitchHistory)
                meanPitch += p;
            meanPitch /= pitchHistory.size();
            
            for (float p : pitchHistory)
                pitchVariance += (p - meanPitch) * (p - meanPitch);
            pitchVariance /= pitchHistory.size();
        }
        
        // Confidence based on pitch stability (lower variance = higher confidence)
        prediction.confidence = jlimit(0.0f, 1.0f, 1.0f - (pitchVariance / (pitch * pitch * 0.01f)));
        
        // Update pitch history
        pitchHistory.erase(pitchHistory.begin());
        pitchHistory.push_back(pitch);
    }
    
    return prediction;
}

void AIModelLoader::performSpectralAnalysis(const float* input, int numSamples, 
                                           std::vector<float>& spectrum)
{
    spectrum.resize(fftSize / 2 + 1, 0.0f);
    
    if (numSamples < fftSize)
    {
        // Zero-pad input if too short
        for (int i = 0; i < fftSize; ++i)
        {
            if (i < numSamples)
                frequencyData[i] = dsp::Complex<float>(input[i], 0.0f);
            else
                frequencyData[i] = dsp::Complex<float>(0.0f, 0.0f);
        }
    }
    else
    {
        // Take first fftSize samples
        for (int i = 0; i < fftSize; ++i)
        {
            frequencyData[i] = dsp::Complex<float>(input[i], 0.0f);
        }
    }
    
    // Apply window
    for (int i = 0; i < fftSize; ++i)
    {
        float window = 0.5f * (1.0f - std::cos(Utils::TWO_PI * i / (fftSize - 1)));
        frequencyData[i] = frequencyData[i] * window;
    }
    
    // Perform FFT
    fft->performFrequencyOnlyForwardTransform(reinterpret_cast<float*>(frequencyData.getData()));
    
    // Calculate magnitude spectrum
    for (int i = 0; i < fftSize / 2 + 1; ++i)
    {
        float real = frequencyData[i].real();
        float imag = frequencyData[i].imag();
        spectrum[i] = std::sqrt(real * real + imag * imag);
    }
}

void AIModelLoader::extractHarmonics(const std::vector<float>& spectrum, float fundamentalFreq,
                                    std::vector<float>& harmonics)
{
    harmonics.resize(16, 0.0f);
    
    if (fundamentalFreq <= 0.0f || spectrum.empty())
        return;
    
    float binWidth = static_cast<float>(currentSampleRate) / fftSize;
    
    for (int h = 1; h <= harmonics.size(); ++h)
    {
        float harmonicFreq = fundamentalFreq * h;
        int bin = static_cast<int>(harmonicFreq / binWidth);
        
        if (bin < spectrum.size())
        {
            // Use peak around the expected harmonic frequency
            float maxMag = 0.0f;
            int searchRange = 3; // Search ±3 bins around expected frequency
            
            for (int b = jmax(0, bin - searchRange); 
                 b <= jmin(static_cast<int>(spectrum.size()) - 1, bin + searchRange); ++b)
            {
                maxMag = jmax(maxMag, spectrum[b]);
            }
            
            harmonics[h - 1] = maxMag;
        }
    }
    
    // Normalize harmonics relative to fundamental
    if (harmonics[0] > 0.0f)
    {
        for (float& harmonic : harmonics)
        {
            harmonic /= harmonics[0];
        }
    }
}

void AIModelLoader::synthesizeHarmonics(float* output, int numSamples, const SynthesisParams& params)
{
    if (!synthesizer)
        return;
    
    float phaseIncrement = Utils::TWO_PI * params.fundamentalFreq / currentSampleRate;
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float harmonicSum = 0.0f;
        
        // Generate harmonics
        for (int h = 0; h < synthesizer->harmonicPhases.size() && h < params.harmonicAmplitudes.size(); ++h)
        {
            if (params.harmonicAmplitudes[h] > 0.0f)
            {
                float harmonicFreq = params.fundamentalFreq * (h + 1);
                float harmonicPhaseInc = phaseIncrement * (h + 1);
                
                synthesizer->harmonicPhases[h] += harmonicPhaseInc;
                if (synthesizer->harmonicPhases[h] > Utils::TWO_PI)
                    synthesizer->harmonicPhases[h] -= Utils::TWO_PI;
                
                harmonicSum += std::sin(synthesizer->harmonicPhases[h]) * params.harmonicAmplitudes[h];
            }
        }
        
        output[sample] = harmonicSum * 0.1f; // Scale down to prevent clipping
    }
}

void AIModelLoader::synthesizeNoise(float* output, int numSamples, float noisiness)
{
    if (!synthesizer)
        return;
    
    for (int i = 0; i < numSamples; ++i)
    {
        // Generate filtered noise
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
    
    // Apply smoothing to reduce jitter
    float smoothed = lastPitchEstimate * (1.0f - pitchSmoothing) + newPitch * pitchSmoothing;
    lastPitchEstimate = smoothed;
    
    return smoothed;
}
