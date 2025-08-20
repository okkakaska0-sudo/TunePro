#include "fftw3.h"
#include <cmath>

// Stub implementations
fftwf_plan fftwf_plan_dft_1d(int n, fftwf_complex* in, fftwf_complex* out, int sign, unsigned flags) {
    // Return a dummy plan
    return reinterpret_cast<fftwf_plan>(1);
}

void fftwf_execute(const fftwf_plan plan) {
    // Stub - in real implementation would perform FFT
}

void fftwf_destroy_plan(fftwf_plan plan) {
    // Stub - cleanup
}

void fftwf_cleanup(void) {
    // Stub - global cleanup
}

namespace FFTW_Stub {
    void simple_fft(std::vector<std::complex<float>>& data) {
        size_t n = data.size();
        if (n <= 1) return;
        
        // Simple DFT implementation (not optimized)
        std::vector<std::complex<float>> temp(n);
        for (size_t k = 0; k < n; ++k) {
            std::complex<float> sum(0, 0);
            for (size_t j = 0; j < n; ++j) {
                float angle = -2.0f * M_PI * k * j / n;
                std::complex<float> w(cos(angle), sin(angle));
                sum += data[j] * w;
            }
            temp[k] = sum;
        }
        data = temp;
    }
    
    void simple_ifft(std::vector<std::complex<float>>& data) {
        size_t n = data.size();
        if (n <= 1) return;
        
        // Conjugate, apply DFT, conjugate and scale
        for (auto& x : data) x = std::conj(x);
        simple_fft(data);
        for (auto& x : data) x = std::conj(x) / static_cast<float>(n);
    }
}