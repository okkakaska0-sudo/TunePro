#include "RubberBandStretcher.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace RubberBand {

struct RubberBandStretcher::Impl {
    size_t sampleRate;
    size_t channels;
    Options options;
    double timeRatio;
    double pitchScale;
    
    // Processing buffers
    std::vector<std::vector<float>> inputBuffer;
    std::vector<std::vector<float>> outputBuffer;
    std::vector<std::vector<float>> stretchBuffer;
    
    // State variables
    size_t inputPos;
    size_t outputPos;
    size_t latency;
    size_t frameSize;
    size_t hopSize;
    
    // Analysis variables
    std::vector<float> window;
    std::vector<std::vector<float>> prevFrame;
    std::vector<float> phases;
    
    Impl(size_t sr, size_t ch, Options opts, double timeR, double pitchS)
        : sampleRate(sr), channels(ch), options(opts), 
          timeRatio(timeR), pitchScale(pitchS),
          inputPos(0), outputPos(0), latency(256)
    {
        frameSize = 2048;
        hopSize = frameSize / 4;
        
        // Initialize buffers
        inputBuffer.resize(channels);
        outputBuffer.resize(channels);
        stretchBuffer.resize(channels);
        prevFrame.resize(channels);
        
        for (size_t c = 0; c < channels; ++c) {
            inputBuffer[c].resize(frameSize * 2, 0.0f);
            outputBuffer[c].resize(frameSize * 4, 0.0f);
            stretchBuffer[c].resize(frameSize * 2, 0.0f);
            prevFrame[c].resize(frameSize, 0.0f);
        }
        
        // Hanning window
        window.resize(frameSize);
        for (size_t i = 0; i < frameSize; ++i) {
            window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (frameSize - 1)));
        }
        
        phases.resize(frameSize / 2 + 1, 0.0f);
    }
    
    void processFrame(size_t channel) {
        // Simple time-domain stretching using overlap-add
        const float* input = inputBuffer[channel].data() + inputPos;
        float* output = outputBuffer[channel].data() + outputPos;
        
        // Apply pitch scaling by resampling
        if (std::abs(pitchScale - 1.0) > 0.001) {
            // Simple linear interpolation for pitch scaling
            for (size_t i = 0; i < hopSize; ++i) {
                double srcIndex = i / pitchScale;
                size_t idx1 = static_cast<size_t>(srcIndex);
                size_t idx2 = idx1 + 1;
                float frac = srcIndex - idx1;
                
                if (idx2 < frameSize) {
                    float sample1 = idx1 < frameSize ? input[idx1] : 0.0f;
                    float sample2 = input[idx2];
                    stretchBuffer[channel][i] = sample1 + frac * (sample2 - sample1);
                } else {
                    stretchBuffer[channel][i] = idx1 < frameSize ? input[idx1] : 0.0f;
                }
            }
        } else {
            // No pitch scaling, direct copy
            std::memcpy(stretchBuffer[channel].data(), input, hopSize * sizeof(float));
        }
        
        // Apply window and overlap-add
        for (size_t i = 0; i < hopSize; ++i) {
            float windowed = stretchBuffer[channel][i] * window[i];
            output[i] += windowed;
        }
    }
};

RubberBandStretcher::RubberBandStretcher(size_t sampleRate, size_t channels,
                                       Options options, double initialTimeRatio,
                                       double initialPitchScale)
    : impl_(std::make_unique<Impl>(sampleRate, channels, options, 
                                  initialTimeRatio, initialPitchScale))
{
}

RubberBandStretcher::~RubberBandStretcher() = default;

void RubberBandStretcher::reset() {
    impl_->inputPos = 0;
    impl_->outputPos = 0;
    
    for (size_t c = 0; c < impl_->channels; ++c) {
        std::fill(impl_->inputBuffer[c].begin(), impl_->inputBuffer[c].end(), 0.0f);
        std::fill(impl_->outputBuffer[c].begin(), impl_->outputBuffer[c].end(), 0.0f);
        std::fill(impl_->stretchBuffer[c].begin(), impl_->stretchBuffer[c].end(), 0.0f);
    }
}

void RubberBandStretcher::setTimeRatio(double ratio) {
    impl_->timeRatio = ratio;
}

void RubberBandStretcher::setPitchScale(double scale) {
    impl_->pitchScale = scale;
}

double RubberBandStretcher::getTimeRatio() const {
    return impl_->timeRatio;
}

double RubberBandStretcher::getPitchScale() const {
    return impl_->pitchScale;
}

size_t RubberBandStretcher::getLatency() const {
    return impl_->latency;
}

void RubberBandStretcher::setTransientsOption(Options options) {
    impl_->options = (impl_->options & ~0x00000F00) | (options & 0x00000F00);
}

void RubberBandStretcher::setDetectorOption(Options options) {
    impl_->options = (impl_->options & ~0x00000C00) | (options & 0x00000C00);
}

void RubberBandStretcher::setPhaseOption(Options options) {
    impl_->options = (impl_->options & ~0x00002000) | (options & 0x00002000);
}

void RubberBandStretcher::setFormantOption(Options options) {
    impl_->options = (impl_->options & ~0x01000000) | (options & 0x01000000);
}

void RubberBandStretcher::setPitchOption(Options options) {
    impl_->options = (impl_->options & ~0x06000000) | (options & 0x06000000);
}

size_t RubberBandStretcher::getSamplesRequired() const {
    return impl_->hopSize;
}

void RubberBandStretcher::setMaxProcessSize(size_t samples) {
    // This is a hint for processing efficiency
}

void RubberBandStretcher::setExpectedInputDuration(size_t samples) {
    // This is used for offline processing optimization
}

size_t RubberBandStretcher::study(const float *const *input, size_t samples, bool final) {
    // In real-time mode, study is the same as process
    process(input, samples, final);
    return samples;
}

void RubberBandStretcher::process(const float *const *input, size_t samples, bool final) {
    for (size_t s = 0; s < samples; ++s) {
        for (size_t c = 0; c < impl_->channels; ++c) {
            impl_->inputBuffer[c][impl_->inputPos] = input[c][s];
        }
        
        impl_->inputPos++;
        
        // Process when we have enough samples
        if (impl_->inputPos >= impl_->hopSize) {
            for (size_t c = 0; c < impl_->channels; ++c) {
                impl_->processFrame(c);
            }
            
            // Shift input buffer
            size_t shift = impl_->hopSize;
            for (size_t c = 0; c < impl_->channels; ++c) {
                std::memmove(impl_->inputBuffer[c].data(), 
                           impl_->inputBuffer[c].data() + shift,
                           (impl_->frameSize - shift) * sizeof(float));
            }
            impl_->inputPos -= shift;
            impl_->outputPos += impl_->hopSize;
        }
    }
}

int RubberBandStretcher::available() const {
    return static_cast<int>(impl_->outputPos);
}

size_t RubberBandStretcher::retrieve(float *const *output, size_t samples) const {
    size_t toRetrieve = std::min(samples, impl_->outputPos);
    
    for (size_t c = 0; c < impl_->channels; ++c) {
        std::memcpy(output[c], impl_->outputBuffer[c].data(), toRetrieve * sizeof(float));
        
        // Shift output buffer
        std::memmove(impl_->outputBuffer[c].data(),
                   impl_->outputBuffer[c].data() + toRetrieve,
                   (impl_->outputBuffer[c].size() - toRetrieve) * sizeof(float));
    }
    
    impl_->outputPos -= toRetrieve;
    return toRetrieve;
}

float RubberBandStretcher::getFrequencyCutoff(int n) const {
    return 0.0f; // Placeholder
}

void RubberBandStretcher::setFrequencyCutoff(int n, float cutoff) {
    // Placeholder
}

size_t RubberBandStretcher::getInputIncrement() const {
    return impl_->hopSize;
}

size_t RubberBandStretcher::getOutputIncrement() const {
    return static_cast<size_t>(impl_->hopSize * impl_->timeRatio);
}

void RubberBandStretcher::calculateStretch() {
    // Placeholder for stretch calculation
}

void RubberBandStretcher::setDebugLevel(int level) {
    // Placeholder
}

void RubberBandStretcher::setDefaultDebugLevel(int level) {
    // Placeholder
}

} // namespace RubberBand