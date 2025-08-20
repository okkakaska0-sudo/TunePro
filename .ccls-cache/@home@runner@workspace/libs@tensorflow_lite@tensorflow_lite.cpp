#include "tensorflow_lite.h"
#include <cstring>
#include <algorithm>

namespace tflite {

Interpreter::Interpreter() = default;

Interpreter::~Interpreter() = default;

bool Interpreter::LoadModelFromBuffer(const char* model_data, size_t model_size) {
    if (!model_data || model_size == 0) return false;
    
    // Initialize input tensor for audio (e.g., 1024 samples)
    auto input = std::make_unique<Tensor>();
    input->shape = {1, 1024};
    input->size = 1024;
    input->data = new float[1024];
    std::fill(input->data, input->data + 1024, 0.0f);
    input_tensors_.push_back(std::move(input));
    
    // Initialize output tensor for pitch/confidence
    auto output = std::make_unique<Tensor>();
    output->shape = {1, 2}; // [frequency, confidence]
    output->size = 2;
    output->data = new float[2];
    std::fill(output->data, output->data + 2, 0.0f);
    output_tensors_.push_back(std::move(output));
    
    model_loaded_ = true;
    return true;
}

bool Interpreter::AllocateTensors() {
    if (!model_loaded_) return false;
    tensors_allocated_ = true;
    return true;
}

Tensor* Interpreter::input_tensor(int index) {
    if (index >= 0 && index < static_cast<int>(input_tensors_.size())) {
        return input_tensors_[index].get();
    }
    return nullptr;
}

Tensor* Interpreter::output_tensor(int index) {
    if (index >= 0 && index < static_cast<int>(output_tensors_.size())) {
        return output_tensors_[index].get();
    }
    return nullptr;
}

bool Interpreter::Invoke() {
    if (!tensors_allocated_ || input_tensors_.empty() || output_tensors_.empty()) {
        return false;
    }
    
    // Simplified pitch detection using autocorrelation
    float* input_data = input_tensors_[0]->data;
    float* output_data = output_tensors_[0]->data;
    
    // Find fundamental frequency using autocorrelation
    float best_correlation = 0.0f;
    int best_period = 0;
    
    const int min_period = 20;  // ~2200 Hz at 44.1kHz
    const int max_period = 400; // ~110 Hz at 44.1kHz
    const int buffer_size = 1024;
    
    for (int period = min_period; period < max_period && period < buffer_size/2; ++period) {
        float correlation = 0.0f;
        for (int i = 0; i < buffer_size - period; ++i) {
            correlation += input_data[i] * input_data[i + period];
        }
        
        if (correlation > best_correlation) {
            best_correlation = correlation;
            best_period = period;
        }
    }
    
    // Convert period to frequency (assuming 44.1kHz sample rate)
    float frequency = best_period > 0 ? 44100.0f / best_period : 0.0f;
    float confidence = std::min(1.0f, best_correlation / 1000.0f);
    
    output_data[0] = frequency;
    output_data[1] = confidence;
    
    return true;
}

std::unique_ptr<FlatBufferModel> FlatBufferModel::BuildFromBuffer(
    const char* caller_owned_buffer, size_t buffer_size) {
    
    auto model = std::make_unique<FlatBufferModel>();
    if (caller_owned_buffer && buffer_size > 0) {
        model->model_data_.resize(buffer_size);
        std::memcpy(model->model_data_.data(), caller_owned_buffer, buffer_size);
        model->initialized_ = true;
    }
    return model;
}

InterpreterBuilder::InterpreterBuilder(const FlatBufferModel& model) : model_(model) {}

bool InterpreterBuilder::operator()(std::unique_ptr<Interpreter>* interpreter) {
    if (!model_.initialized()) return false;
    
    auto interp = std::make_unique<Interpreter>();
    if (interp->LoadModelFromBuffer(nullptr, 0) && interp->AllocateTensors()) {
        *interpreter = std::move(interp);
        return true;
    }
    return false;
}

} // namespace tflite