#include "PitchCorrectionEngine.h"
#include "Utils.h"
#include <algorithm>

// Functions now available globally from JuceHeader.h
#include <cstring>

PitchCorrectionEngine::PitchCorrectionEngine()
{
    // Initialize grain buffers
    grains.resize(maxGrains);
}

PitchCorrectionEngine::~PitchCorrectionEngine()
{
    releaseGrains();
}

void PitchCorrectionEngine::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    // Initialize buffers
    analysisBuffer.setSize(1, samplesPerBlock * 2); // Double buffer for pitch detection
    correlationBuffer.setSize(1, samplesPerBlock * 2);
    windowBuffer.resize(samplesPerBlock * 2);
    
    // Initialize FFT
    fft = std::make_unique<dsp::FFT>(fftOrder);
    window = std::make_unique<dsp::WindowingFunction<float>>(fftSize, dsp::WindowingFunction<float>::hann);
    frequencyData.allocate(fftSize * 2, true);
    
    // Initialize overlap buffer
    overlapBuffer.setSize(1, overlapSize);
    overlapBuffer.clear();
    overlapPosition = 0;
    
    // Initialize grains
    initializeGrains();
}

void PitchCorrectionEngine::reset()
{
    analysisBuffer.clear();
    correlationBuffer.clear();
    overlapBuffer.clear();
    overlapPosition = 0;
    currentGrain = 0;
    
    // Reset all grains
    for (auto& grain : grains)
    {
        grain.active = false;
        grain.position = 0;
        grain.phase = 0.0f;
        grain.amplitude = 0.0f;
    }
}

void PitchCorrectionEngine::detectPitch(const float* input, int numSamples, float* pitchOutput)
{
    // Use autocorrelation for basic pitch detection
    for (int i = 0; i < numSamples; i += 256) // Process in chunks
    {
        int chunkSize = jmin(256, numSamples - i);
        float pitch = detectPitchAutocorrelation(&input[i], chunkSize);
        
        // Fill the output buffer with detected pitch
        for (int j = 0; j < chunkSize; ++j)
        {
            if (i + j < numSamples)
                pitchOutput[i + j] = pitch;
        }
    }
}

void PitchCorrectionEngine::detectPitchAdvanced(const float* input, int numSamples, float* pitchOutput)
{
    // Use YIN algorithm for more accurate pitch detection
    for (int i = 0; i < numSamples; i += 512) // Larger chunks for better accuracy
    {
        int chunkSize = jmin(512, numSamples - i);
        float pitch = detectPitchYIN(&input[i], chunkSize);
        
        // Fill the output buffer with detected pitch
        for (int j = 0; j < chunkSize; ++j)
        {
            if (i + j < numSamples)
                pitchOutput[i + j] = pitch;
        }
    }
}

void PitchCorrectionEngine::correctPitch(float* audio, int numSamples, float targetFreq, float speed, float amount)
{
    // Detect current pitch
    std::vector<float> currentPitches(numSamples);
    detectPitch(audio, numSamples, currentPitches.data());
    
    // Apply smooth pitch correction
    for (int i = 0; i < numSamples; ++i)
    {
        float currentPitch = currentPitches[i];
        
        if (currentPitch > 20.0f && currentPitch < 5000.0f) // Valid pitch range
        {
            float pitchRatio = targetFreq / currentPitch;
            float correction = (pitchRatio - 1.0f) * amount * 0.01f * speed * 0.01f;
            
            if (std::abs(correction) > 0.01f)
            {
                // Apply gradual pitch shifting using granular synthesis
                pitchShiftGranular(&audio[i], jmin(grainSize, numSamples - i), 1.0f + correction);
            }
        }
    }
}

void PitchCorrectionEngine::correctPitchHard(float* audio, int numSamples, float targetFreq, float speed, float amount)
{
    // Detect current pitch
    std::vector<float> currentPitches(numSamples);
    detectPitch(audio, numSamples, currentPitches.data());
    
    // Apply aggressive pitch correction
    for (int i = 0; i < numSamples; ++i)
    {
        float currentPitch = currentPitches[i];
        
        if (currentPitch > 20.0f && currentPitch < 5000.0f)
        {
            float pitchRatio = targetFreq / currentPitch;
            
            // Hard correction - immediate snap to target
            if (std::abs(pitchRatio - 1.0f) > 0.005f && amount > 0.1f)
            {
                // Calculate hard correction factor
                float hardCorrection = pitchRatio - 1.0f;
                hardCorrection *= amount * 0.01f;
                hardCorrection = jlimit(-0.5f, 0.5f, hardCorrection); // Limit extreme corrections
                
                // Apply immediate pitch shift
                pitchShiftPSOLA(&audio[i], jmin(1024, numSamples - i), 1.0f + hardCorrection);
            }
        }
    }
}

void PitchCorrectionEngine::correctPitchAI(float* audio, int numSamples, float targetFreq, float speed, float amount)
{
    // Use advanced pitch detection for AI mode
    std::vector<float> currentPitches(numSamples);
    detectPitchAdvanced(audio, numSamples, currentPitches.data());
    
    // Apply AI-style correction with formant preservation
    for (int i = 0; i < numSamples; i += 256)
    {
        int chunkSize = jmin(256, numSamples - i);
        float currentPitch = currentPitches[i];
        
        if (currentPitch > 20.0f && currentPitch < 5000.0f)
        {
            float pitchRatio = targetFreq / currentPitch;
            
            if (std::abs(pitchRatio - 1.0f) > 0.01f)
            {
                // Apply spectral pitch shifting with formant preservation
                pitchShiftSpectral(&audio[i], chunkSize, pitchRatio);
                
                // Preserve formants for natural sound
                preserveFormants(&audio[i], chunkSize, pitchRatio);
            }
        }
    }
}

float PitchCorrectionEngine::calculateRMS(const float* buffer, int numSamples)
{
    if (numSamples <= 0)
        return 0.0f;
    
    float sum = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        sum += buffer[i] * buffer[i];
    }
    
    return std::sqrt(sum / numSamples);
}

void PitchCorrectionEngine::performFFT(const float* input, std::vector<float>& magnitudeOutput)
{
    if (magnitudeOutput.size() != fftSize / 2 + 1)
        magnitudeOutput.resize(fftSize / 2 + 1);
    
    // Copy input to frequency data (real part)
    for (int i = 0; i < fftSize; ++i)
    {
        if (i < fftSize)
        {
            frequencyData[i].real(i < fftSize ? input[i] : 0.0f);
            frequencyData[i].imag(0.0f);
        }
    }
    
    // Apply window
    window->multiplyWithWindowingTable(reinterpret_cast<float*>(frequencyData.getData()), fftSize);
    
    // Perform FFT
    fft->performFrequencyOnlyForwardTransform(reinterpret_cast<float*>(frequencyData.getData()));
    
    // Calculate magnitude spectrum
    for (int i = 0; i < fftSize / 2 + 1; ++i)
    {
        float real = frequencyData[i].real();
        float imag = frequencyData[i].imag();
        magnitudeOutput[i] = std::sqrt(real * real + imag * imag);
    }
}

void PitchCorrectionEngine::performIFFT(const std::vector<float>& magnitudeInput, float* output)
{
    // Convert magnitude back to complex spectrum (phase is lost)
    for (int i = 0; i < fftSize / 2 + 1; ++i)
    {
        frequencyData[i].real(magnitudeInput[i]);
        frequencyData[i].imag(0.0f); // Zero phase for simplicity
    }
    
    // Mirror for negative frequencies
    for (int i = fftSize / 2 + 1; i < fftSize; ++i)
    {
        frequencyData[i] = std::conj(frequencyData[fftSize - i]);
    }
    
    // Perform IFFT
    fft->performRealOnlyInverseTransform(reinterpret_cast<float*>(frequencyData.getData()));
    
    // Copy result to output
    for (int i = 0; i < fftSize; ++i)
    {
        output[i] = frequencyData[i].real();
    }
}

float PitchCorrectionEngine::detectPitchAutocorrelation(const float* input, int numSamples)
{
    if (numSamples < 100)
        return 0.0f;
    
    // Apply window to input
    auto* analysisData = analysisBuffer.getWritePointer(0);
    for (int i = 0; i < numSamples; ++i)
    {
        float windowVal = 0.5f * (1.0f - std::cos(Utils::TWO_PI * i / (numSamples - 1))); // Hann window
        analysisData[i] = input[i] * windowVal;
    }
    
    // Autocorrelation
    int minPeriod = static_cast<int>(currentSampleRate / 1000.0); // Min 1000 Hz
    int maxPeriod = static_cast<int>(currentSampleRate / 80.0);   // Max 80 Hz
    
    float maxCorrelation = 0.0f;
    int bestPeriod = minPeriod;
    
    for (int period = minPeriod; period < std::min(maxPeriod, numSamples / 2); ++period)
    {
        float correlation = 0.0f;
        float energy1 = 0.0f;
        float energy2 = 0.0f;
        
        int samples = numSamples - period;
        for (int i = 0; i < samples; ++i)
        {
            correlation += analysisData[i] * analysisData[i + period];
            energy1 += analysisData[i] * analysisData[i];
            energy2 += analysisData[i + period] * analysisData[i + period];
        }
        
        // Normalized correlation
        float normalizedCorr = correlation / std::sqrt(energy1 * energy2 + 1e-10f);
        
        if (normalizedCorr > maxCorrelation)
        {
            maxCorrelation = normalizedCorr;
            bestPeriod = period;
        }
    }
    
    // Return frequency if correlation is strong enough
    if (maxCorrelation > 0.3f)
    {
        return static_cast<float>(currentSampleRate) / bestPeriod;
    }
    
    return 0.0f;
}

float PitchCorrectionEngine::detectPitchYIN(const float* input, int numSamples)
{
    if (numSamples < 200)
        return 0.0f;
    
    int maxTau = std::min(numSamples / 2, static_cast<int>(currentSampleRate / 80.0));
    std::vector<float> yinBuffer(maxTau);
    
    // Step 1: Difference function
    yinBuffer[0] = 1.0f;
    float runningSum = 0.0f;
    
    for (int tau = 1; tau < maxTau; ++tau)
    {
        yinBuffer[tau] = 0.0f;
        
        for (int i = 0; i < numSamples - tau; ++i)
        {
            float delta = input[i] - input[i + tau];
            yinBuffer[tau] += delta * delta;
        }
        
        // Step 2: Cumulative mean normalized difference
        runningSum += yinBuffer[tau];
        yinBuffer[tau] *= tau / runningSum;
    }
    
    // Step 3: Absolute threshold
    const float threshold = 0.1f;
    int tau = 1;
    
    while (tau < maxTau - 1 && yinBuffer[tau] > threshold)
        ++tau;
    
    if (tau == maxTau - 1)
        return 0.0f;
    
    // Step 4: Parabolic interpolation
    float x0 = (tau < 1) ? tau : tau - 1;
    float x2 = (tau + 1 < maxTau) ? tau + 1 : tau;
    
    float a = (yinBuffer[static_cast<int>(x0)] + yinBuffer[static_cast<int>(x2)] - 2 * yinBuffer[tau]) / 2;
    float b = (yinBuffer[static_cast<int>(x2)] - yinBuffer[static_cast<int>(x0)]) / 2;
    
    float betterTau = tau;
    if (a != 0.0f)
        betterTau = tau - b / (2 * a);
    
    return static_cast<float>(currentSampleRate) / betterTau;
}

float PitchCorrectionEngine::detectPitchSpectral(const float* input, int numSamples)
{
    std::vector<float> spectrum(fftSize / 2 + 1);
    performFFT(input, spectrum);
    
    // Find the spectral peak
    int peakBin = 0;
    float maxMagnitude = 0.0f;
    
    // Look for peaks in the vocal range (80 Hz - 2000 Hz)
    int minBin = static_cast<int>(80.0f * fftSize / currentSampleRate);
    int maxBin = static_cast<int>(2000.0f * fftSize / currentSampleRate);
    
    for (int i = minBin; i < std::min(maxBin, static_cast<int>(spectrum.size())); ++i)
    {
        if (spectrum[i] > maxMagnitude)
        {
            maxMagnitude = spectrum[i];
            peakBin = i;
        }
    }
    
    if (peakBin > 0 && maxMagnitude > 0.1f)
    {
        // Parabolic interpolation for better frequency resolution
        float y1 = peakBin > 0 ? spectrum[peakBin - 1] : 0.0f;
        float y2 = spectrum[peakBin];
        float y3 = peakBin < spectrum.size() - 1 ? spectrum[peakBin + 1] : 0.0f;
        
        float a = (y1 + y3 - 2 * y2) / 2;
        float b = (y3 - y1) / 2;
        
        float peakOffset = 0.0f;
        if (a != 0.0f)
            peakOffset = -b / (2 * a);
        
        float interpolatedBin = peakBin + peakOffset;
        return interpolatedBin * static_cast<float>(currentSampleRate) / fftSize;
    }
    
    return 0.0f;
}

void PitchCorrectionEngine::pitchShiftPSOLA(float* audio, int numSamples, float pitchRatio)
{
    // Simplified PSOLA implementation
    if (std::abs(pitchRatio - 1.0f) < 0.01f)
        return;
    
    int frameSize = 512;
    int hopSize = frameSize / 4;
    
    for (int pos = 0; pos < numSamples - frameSize; pos += hopSize)
    {
        int frameEnd = std::min(pos + frameSize, numSamples);
        int currentFrameSize = frameEnd - pos;
        
        // Apply Hann window
        for (int i = 0; i < currentFrameSize; ++i)
        {
            float window = 0.5f * (1.0f - std::cos(Utils::TWO_PI * i / (currentFrameSize - 1)));
            audio[pos + i] *= window;
        }
        
        // Simple time-domain pitch shifting
        if (pitchRatio > 1.0f)
        {
            // Higher pitch - compress time
            for (int i = 0; i < currentFrameSize; ++i)
            {
                float sourceIndex = i / pitchRatio;
                int index1 = static_cast<int>(sourceIndex);
                int index2 = index1 + 1;
                
                if (index2 < currentFrameSize)
                {
                    float frac = sourceIndex - index1;
                    audio[pos + i] = audio[pos + index1] * (1.0f - frac) + audio[pos + index2] * frac;
                }
            }
        }
        else if (pitchRatio < 1.0f)
        {
            // Lower pitch - expand time
            for (int i = currentFrameSize - 1; i >= 0; --i)
            {
                float sourceIndex = i * pitchRatio;
                int index1 = static_cast<int>(sourceIndex);
                int index2 = index1 + 1;
                
                if (index2 < currentFrameSize && index1 >= 0)
                {
                    float frac = sourceIndex - index1;
                    audio[pos + i] = audio[pos + index1] * (1.0f - frac) + audio[pos + index2] * frac;
                }
            }
        }
    }
}

void PitchCorrectionEngine::pitchShiftGranular(float* audio, int numSamples, float pitchRatio)
{
    // Get available grain
    GrainData* grain = getNextGrain();
    if (!grain || !grain->buffer)
        return;
    
    // Copy input to grain buffer
    int grainSamples = std::min(grain->size, numSamples);
    std::memcpy(grain->buffer, audio, grainSamples * sizeof(float));
    
    // Process grain
    processGrain(*grain, pitchRatio, 1.0f);
    
    // Apply window and copy back
    for (int i = 0; i < grainSamples; ++i)
    {
        float window = 0.5f * (1.0f - std::cos(Utils::TWO_PI * i / (grainSamples - 1)));
        audio[i] = grain->buffer[i] * window;
    }
}

void PitchCorrectionEngine::pitchShiftSpectral(float* audio, int numSamples, float pitchRatio)
{
    if (numSamples < fftSize)
        return;
    
    // Perform FFT
    std::vector<float> spectrum(fftSize / 2 + 1);
    performFFT(audio, spectrum);
    
    // Shift spectrum
    std::vector<float> shiftedSpectrum(fftSize / 2 + 1, 0.0f);
    
    for (int i = 0; i < spectrum.size(); ++i)
    {
        int targetBin = static_cast<int>(i * pitchRatio);
        if (targetBin < shiftedSpectrum.size())
        {
            shiftedSpectrum[targetBin] = spectrum[i];
        }
    }
    
    // Perform IFFT
    performIFFT(shiftedSpectrum, audio);
}

void PitchCorrectionEngine::preserveFormants(float* audio, int numSamples, float pitchRatio)
{
    // Simple formant preservation by envelope extraction and reapplication
    std::vector<float> formantEnvelope;
    extractFormantEnvelope(audio, numSamples, formantEnvelope);
    applyFormantEnvelope(audio, numSamples, formantEnvelope);
}

void PitchCorrectionEngine::extractFormantEnvelope(const float* input, int numSamples, std::vector<float>& formants)
{
    // Simplified formant extraction using spectral envelope
    std::vector<float> spectrum(fftSize / 2 + 1);
    performFFT(input, spectrum);
    
    formants.resize(spectrum.size());
    
    // Smooth the spectrum to get formant envelope
    int smoothingWindow = 8;
    for (int i = 0; i < spectrum.size(); ++i)
    {
        float sum = 0.0f;
        int count = 0;
        
        for (int j = std::max(0, i - smoothingWindow); j <= std::min(static_cast<int>(spectrum.size()) - 1, i + smoothingWindow); ++j)
        {
            sum += spectrum[j];
            ++count;
        }
        
        formants[i] = count > 0 ? sum / count : 0.0f;
    }
}

void PitchCorrectionEngine::applyFormantEnvelope(float* audio, int numSamples, const std::vector<float>& formants)
{
    // Apply formant envelope to the audio
    std::vector<float> spectrum(fftSize / 2 + 1);
    performFFT(audio, spectrum);
    
    // Apply formant envelope
    for (int i = 0; i < std::min(spectrum.size(), formants.size()); ++i)
    {
        if (formants[i] > 0.0f && spectrum[i] > 0.0f)
        {
            spectrum[i] *= formants[i] / (spectrum[i] + 1e-10f);
        }
    }
    
    // Convert back to time domain
    performIFFT(spectrum, audio);
}

void PitchCorrectionEngine::initializeGrains()
{
    grainBuffers.setSize(maxGrains, grainSize);
    grainBuffers.clear();
    
    for (int i = 0; i < maxGrains; ++i)
    {
        grains[i].buffer = grainBuffers.getWritePointer(i);
        grains[i].size = grainSize;
        grains[i].position = 0;
        grains[i].phase = 0.0f;
        grains[i].amplitude = 0.0f;
        grains[i].active = false;
    }
}

void PitchCorrectionEngine::releaseGrains()
{
    for (auto& grain : grains)
    {
        grain.buffer = nullptr;
        grain.active = false;
    }
}

PitchCorrectionEngine::GrainData* PitchCorrectionEngine::getNextGrain()
{
    // Round-robin grain selection
    GrainData* grain = &grains[currentGrain];
    currentGrain = (currentGrain + 1) % maxGrains;
    
    // Reset grain
    grain->position = 0;
    grain->phase = 0.0f;
    grain->amplitude = 1.0f;
    grain->active = true;
    
    return grain;
}

void PitchCorrectionEngine::processGrain(GrainData& grain, float pitchRatio, float speed)
{
    if (!grain.active || !grain.buffer)
        return;
    
    // Simple pitch shifting by sample rate conversion
    for (int i = 0; i < grain.size; ++i)
    {
        float sourceIndex = i / pitchRatio;
        int index1 = static_cast<int>(sourceIndex);
        int index2 = std::min(index1 + 1, grain.size - 1);
        
        if (index1 < grain.size)
        {
            float frac = sourceIndex - index1;
            grain.buffer[i] = grain.buffer[index1] * (1.0f - frac) + grain.buffer[index2] * frac;
        }
    }
}
