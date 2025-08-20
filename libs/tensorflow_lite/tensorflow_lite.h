#pragma once

// TensorFlow Lite C++ API for AI Model Inference
// This is a simplified header for the MarsiAutoTune project
// Full TensorFlow Lite implementation would be much larger

#include <vector>
#include <string>
#include <memory>

namespace tflite {

// Forward declarations
class Interpreter;
class Model;

// Error codes
enum class Status {
    kOk = 0,
    kError = 1
};

// Tensor types
enum class TensorType {
    kFloat32 = 1,
    kInt32 = 2,
    kUInt8 = 3,
    kInt64 = 4
};

// Tensor structure
struct TensorInfo {
    std::vector<int> shape;
    TensorType type;
    std::string name;
};

// Main interpreter class
class Interpreter {
public:
    Interpreter();
    ~Interpreter();
    
    // Model loading and setup
    Status AllocateTensors();
    Status Invoke();
    
    // Input/Output access
    float* typed_input_tensor(int tensor_index);
    float* typed_output_tensor(int tensor_index);
    
    // Tensor information
    int inputs_size() const;
    int outputs_size() const;
    TensorInfo* input_tensor(int index);
    TensorInfo* output_tensor(int index);
    
    // Tensor manipulation
    Status ResizeInputTensor(int tensor_index, const std::vector<int>& dims);
    Status SetTensorParametersReadWrite(int tensor_index, TensorType type,
                                       const char* name,
                                       const std::vector<int>& dims,
                                       const void* data, size_t bytes);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Model loading utilities
class FlatBufferModel {
public:
    static std::unique_ptr<FlatBufferModel> BuildFromFile(const char* filename);
    static std::unique_ptr<FlatBufferModel> BuildFromBuffer(const char* buffer, size_t buffer_size);
    
    bool initialized() const;
    const void* allocation() const;

private:
    FlatBufferModel();
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Interpreter builder
class InterpreterBuilder {
public:
    InterpreterBuilder(const FlatBufferModel& model);
    ~InterpreterBuilder();
    
    Status operator()(std::unique_ptr<Interpreter>* interpreter);
    Status operator()(std::unique_ptr<Interpreter>* interpreter, int num_threads);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Registration utilities (for custom ops if needed)
void RegisterBuiltinOps();

} // namespace tflite

// C API wrappers for compatibility
extern "C" {
    typedef struct TfLiteModel TfLiteModel;
    typedef struct TfLiteInterpreter TfLiteInterpreter;
    typedef struct TfLiteTensor TfLiteTensor;
    
    TfLiteModel* TfLiteModelCreateFromFile(const char* model_path);
    void TfLiteModelDelete(TfLiteModel* model);
    
    TfLiteInterpreter* TfLiteInterpreterCreate(const TfLiteModel* model);
    void TfLiteInterpreterDelete(TfLiteInterpreter* interpreter);
    
    int TfLiteInterpreterAllocateTensors(TfLiteInterpreter* interpreter);
    int TfLiteInterpreterInvoke(TfLiteInterpreter* interpreter);
    
    TfLiteTensor* TfLiteInterpreterGetInputTensor(const TfLiteInterpreter* interpreter, int input_index);
    const TfLiteTensor* TfLiteInterpreterGetOutputTensor(const TfLiteInterpreter* interpreter, int output_index);
    
    float* TfLiteTensorData(TfLiteTensor* tensor);
    int TfLiteTensorByteSize(const TfLiteTensor* tensor);
}