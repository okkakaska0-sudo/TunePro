#pragma once
#include <vector>
#include <string>
#include <memory>

namespace OnnxRuntime {
    class Session {
    public:
        Session(const std::string& modelPath);
        ~Session();
        
        std::vector<float> run(const std::vector<float>& input);
        std::vector<float> runVocalSynthesis(const std::vector<float>& pitchInput, const std::vector<float>& audioInput);
        bool isLoaded() const { return loaded; }
        
    private:
        bool loaded = false;
        std::string modelPath;
        void processAudioWithDDSP(std::vector<float>& audio, const std::vector<float>& pitch);
    };
    
    class Environment {
    public:
        static void initialize();
        static bool isInitialized();
    private:
        static bool initialized;
    };
}