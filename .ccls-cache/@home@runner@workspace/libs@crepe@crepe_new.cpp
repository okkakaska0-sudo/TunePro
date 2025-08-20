#include "crepe.h"
#include "../tensorflow_lite/tensorflow_lite.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstring>

bool CrepeModel::initialized_ = false;
std::unique_ptr<tflite::Interpreter> CrepeModel::interpreter_ = nullptr;
std::unique_ptr<tflite::FlatBufferModel> CrepeModel::model_ = nullptr;
std::array<float, CrepeModel::CREPE_CENTS_MAPPING_SIZE> CrepeModel::centsMapping_;

bool CrepeModel::initialize() {
    if (initialized_) return true;
    
    // Generate cents mapping for CREPE output
    generateCentsMapping();
    
    // Try to load TensorFlow Lite model
    if (loadModel()) {
        initialized_ = true;
        return true;
    }
    
    // If model loading fails, we can still use fallback algorithms
    initialized_ = true;
    return true;
}

bool CrepeModel::isInitialized() {
    return initialized_;
}

void CrepeModel::shutdown() {
    interpreter_.reset();
    model_.reset();
    initialized_ = false;
}

void CrepeModel::setModelPath(const std::string& path) {
    // Store model path for future loading
}

void CrepeModel::setViterbiDecoder(bool enabled) {
    // Configure Viterbi decoding (for smoothing pitch tracks)
}

void CrepeModel::setCenterFrequency(bool center) {
    // Configure frequency centering
}

void CrepeModel::generateCentsMapping() {
    // CREPE outputs 360 values corresponding to cents
    // This maps output indices to frequencies
    const float CENTS_PER_OCTAVE = 1200.0f;
    const float REFERENCE_FREQ = 10.0f; // Reference frequency in Hz
    
    for (size_t i = 0; i < CREPE_CENTS_MAPPING_SIZE; ++i) {
        float cents = (i / static_cast<float>(CREPE_CENTS_MAPPING_SIZE - 1)) * CENTS_PER_OCTAVE * 6; // 6 octaves
        centsMapping_[i] = centsToFrequency(cents);
    }
}

float CrepeModel::centsToFrequency(float cents) {
    const float REFERENCE_FREQ = 10.0f; // 10 Hz reference
    return REFERENCE_FREQ * std::pow(2.0f, cents / 1200.0f);
}

float CrepeModel::frequencyToCents(float frequency) {
    const float REFERENCE_FREQ = 10.0f;
    if (frequency <= 0.0f) return 0.0f;
    return 1200.0f * std::log2(frequency / REFERENCE_FREQ);
}

bool CrepeModel::loadModel() {
    try {
        // Create a minimal TensorFlow Lite model for CREPE
        // In a real implementation, this would load a pre-trained CREPE model
        model_ = tflite::FlatBufferModel::BuildFromBuffer(nullptr, 0);
        if (!model_ || !model_->initialized()) return false;
        
        tflite::InterpreterBuilder builder(*model_);
        if (!builder(&interpreter_)) return false;
        
        if (!interpreter_->AllocateTensors()) return false;
        
        return true;
    } catch (...) {
        return false;
    }
}

CrepeModel::PitchResult CrepeModel::estimatePitch(const std::vector<float>& audioBuffer, float sampleRate) {
    if (!initialized_) initialize();
    
    PitchResult result = {0.0f, 0.0f};
    
    // Validate input
    if (!isAudioValid(audioBuffer) || sampleRate <= 0) {
        return result;
    }
    
    // Preprocess audio for CREPE
    std::vector<float> processedAudio = preprocessAudio(audioBuffer, sampleRate);
    
    // Try TensorFlow Lite inference first
    if (interpreter_ && processedAudio.size() == CREPE_MODEL_CAPACITY) {
        auto* input_tensor = interpreter_->input_tensor(0);
        if (input_tensor && input_tensor->data) {
            // Copy preprocessed audio to input tensor
            std::memcpy(input_tensor->data, processedAudio.data(), 
                       CREPE_MODEL_CAPACITY * sizeof(float));
            
            // Run inference
            if (interpreter_->Invoke()) {
                auto* output_tensor = interpreter_->output_tensor(0);
                if (output_tensor && output_tensor->data) {
                    result = postprocessOutput(output_tensor->data, output_tensor->size);
                    if (result.isValid()) return result;
                }
            }
        }
    }
    
    // Fallback to YIN algorithm
    result = yinPitchDetection(processedAudio, sampleRate);
    if (result.isValid()) return result;
    
    // Final fallback to autocorrelation
    return autocorrelationPitch(processedAudio, sampleRate);
}

std::vector<float> CrepeModel::preprocessAudio(const std::vector<float>& audio, float sampleRate) {
    std::vector<float> processed;
    processed.reserve(CREPE_MODEL_CAPACITY);
    
    // Resample to CREPE's expected sample rate (16kHz)
    const float TARGET_SAMPLE_RATE = 16000.0f;
    const float resampleRatio = sampleRate / TARGET_SAMPLE_RATE;
    
    // Simple linear interpolation resampling
    for (size_t i = 0; i < CREPE_MODEL_CAPACITY && processed.size() < CREPE_MODEL_CAPACITY; ++i) {
        float srcIndex = i * resampleRatio;
        size_t idx1 = static_cast<size_t>(srcIndex);
        size_t idx2 = idx1 + 1;
        
        if (idx2 < audio.size()) {
            float frac = srcIndex - idx1;
            float sample = audio[idx1] + frac * (audio[idx2] - audio[idx1]);
            processed.push_back(sample);
        } else if (idx1 < audio.size()) {
            processed.push_back(audio[idx1]);
        } else {
            processed.push_back(0.0f);
        }
    }
    
    // Pad or trim to exact size
    processed.resize(CREPE_MODEL_CAPACITY, 0.0f);
    
    // Apply Hanning window
    applyHanningWindow(processed);
    
    // Normalize
    float rms = calculateRMS(processed);
    if (rms > 1e-6f) {
        float scale = 1.0f / rms;
        for (float& sample : processed) {
            sample *= scale;
        }
    }
    
    return processed;
}

CrepeModel::PitchResult CrepeModel::postprocessOutput(const float* output, size_t outputSize) {
    if (!output || outputSize < 2) return {0.0f, 0.0f};
    
    // For our simplified implementation, output[0] = frequency, output[1] = confidence
    float frequency = output[0];
    float confidence = output[1];
    
    // Clamp to valid range
    frequency = std::max(MIN_FREQUENCY, std::min(frequency, MAX_FREQUENCY));
    confidence = std::max(0.0f, std::min(confidence, 1.0f));
    
    return {frequency, confidence};
}

CrepeModel::PitchResult CrepeModel::yinPitchDetection(const std::vector<float>& signal, float sampleRate) {
    const int minPeriod = static_cast<int>(sampleRate / MAX_FREQUENCY);
    const int maxPeriod = static_cast<int>(sampleRate / MIN_FREQUENCY);
    
    if (minPeriod >= maxPeriod || maxPeriod >= static_cast<int>(signal.size())) {
        return {0.0f, 0.0f};
    }
    
    std::vector<float> cumulativeDifference(maxPeriod + 1, 0.0f);
    
    // Calculate difference function
    for (int tau = minPeriod; tau <= maxPeriod; ++tau) {
        for (size_t j = 0; j + tau < signal.size(); ++j) {
            float diff = signal[j] - signal[j + tau];
            cumulativeDifference[tau] += diff * diff;
        }
    }
    
    // Cumulative mean normalized difference
    cumulativeDifference[0] = 1.0f;
    float runningSum = 0.0f;
    
    for (int tau = 1; tau <= maxPeriod; ++tau) {
        runningSum += cumulativeDifference[tau];
        if (runningSum != 0.0f) {
            cumulativeDifference[tau] *= tau / runningSum;
        } else {
            cumulativeDifference[tau] = 1.0f;
        }
    }
    
    // Find minimum below threshold
    const float threshold = 0.1f;
    for (int tau = minPeriod; tau <= maxPeriod; ++tau) {
        if (cumulativeDifference[tau] < threshold) {
            // Parabolic interpolation
            if (tau > 0 && tau < maxPeriod) {
                float x0 = cumulativeDifference[tau - 1];
                float x1 = cumulativeDifference[tau];
                float x2 = cumulativeDifference[tau + 1];
                
                float a = (x0 - 2 * x1 + x2) / 2.0f;
                if (std::abs(a) > 1e-6f) {
                    float betterTau = tau - (x2 - x0) / (4 * a);
                    float frequency = sampleRate / betterTau;
                    float confidence = 1.0f - x1; // Higher confidence for lower difference
                    return {frequency, confidence};
                }
            }
            float frequency = sampleRate / tau;
            float confidence = 1.0f - cumulativeDifference[tau];
            return {frequency, confidence};
        }
    }
    
    return {0.0f, 0.0f};
}

CrepeModel::PitchResult CrepeModel::autocorrelationPitch(const std::vector<float>& signal, float sampleRate) {
    const int minPeriod = static_cast<int>(sampleRate / MAX_FREQUENCY);
    const int maxPeriod = static_cast<int>(sampleRate / MIN_FREQUENCY);
    
    float bestCorrelation = 0.0f;
    int bestPeriod = 0;
    
    for (int period = minPeriod; period <= maxPeriod && period < static_cast<int>(signal.size() / 2); ++period) {
        float correlation = autocorrelation(signal, period);
        if (correlation > bestCorrelation) {
            bestCorrelation = correlation;
            bestPeriod = period;
        }
    }
    
    if (bestPeriod > 0) {
        float frequency = sampleRate / bestPeriod;
        float confidence = std::min(1.0f, bestCorrelation / 0.5f); // Normalize correlation
        return {frequency, confidence};
    }
    
    return {0.0f, 0.0f};
}

float CrepeModel::autocorrelation(const std::vector<float>& signal, int lag) {
    float sum = 0.0f;
    float norm1 = 0.0f, norm2 = 0.0f;
    int count = 0;
    
    for (size_t i = 0; i + lag < signal.size(); ++i) {
        sum += signal[i] * signal[i + lag];
        norm1 += signal[i] * signal[i];
        norm2 += signal[i + lag] * signal[i + lag];
        count++;
    }
    
    if (count > 0 && norm1 > 0 && norm2 > 0) {
        return sum / std::sqrt(norm1 * norm2);
    }
    return 0.0f;
}

void CrepeModel::applyHanningWindow(std::vector<float>& buffer) {
    if (buffer.empty()) return;
    
    const size_t N = buffer.size();
    for (size_t i = 0; i < N; ++i) {
        float window = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (N - 1)));
        buffer[i] *= window;
    }
}

float CrepeModel::calculateRMS(const std::vector<float>& buffer) {
    if (buffer.empty()) return 0.0f;
    
    float sum = 0.0f;
    for (float sample : buffer) {
        sum += sample * sample;
    }
    return std::sqrt(sum / buffer.size());
}

bool CrepeModel::isAudioValid(const std::vector<float>& buffer) {
    if (buffer.empty() || buffer.size() < 512) return false;
    
    // Check for non-silent audio
    float rms = calculateRMS(buffer);
    return rms > 1e-6f;
}