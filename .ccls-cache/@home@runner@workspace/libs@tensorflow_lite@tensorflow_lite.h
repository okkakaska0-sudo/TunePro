#pragma once
#include <vector>
#include <memory>
#include <string>

namespace tflite {

// TensorFlow Lite minimal interface for audio processing
struct Tensor {
    float* data;
    size_t size;
    std::vector<int> shape;
};

class Interpreter {
public:
    Interpreter();
    ~Interpreter();
    
    // Model loading and setup
    bool LoadModelFromBuffer(const char* model_data, size_t model_size);
    bool AllocateTensors();
    
    // Input/Output access
    Tensor* input_tensor(int index);
    Tensor* output_tensor(int index);
    
    // Inference
    bool Invoke();
    
    // Model info
    size_t inputs_size() const { return input_tensors_.size(); }
    size_t outputs_size() const { return output_tensors_.size(); }
    
private:
    std::vector<std::unique_ptr<Tensor>> input_tensors_;
    std::vector<std::unique_ptr<Tensor>> output_tensors_;
    bool model_loaded_ = false;
    bool tensors_allocated_ = false;
};

// Model builder utility
class FlatBufferModel {
public:
    static std::unique_ptr<FlatBufferModel> BuildFromBuffer(
        const char* caller_owned_buffer, size_t buffer_size);
    
    bool initialized() const { return initialized_; }
    
private:
    bool initialized_ = false;
    std::vector<char> model_data_;
};

// Interpreter builder
class InterpreterBuilder {
public:
    InterpreterBuilder(const FlatBufferModel& model);
    bool operator()(std::unique_ptr<Interpreter>* interpreter);
    
private:
    const FlatBufferModel& model_;
};

} // namespace tflite