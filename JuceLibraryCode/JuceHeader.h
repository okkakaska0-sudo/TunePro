#pragma once

// JUCE Library stub header for MarsiAutoTune development
// This provides basic definitions for development - full JUCE will be used on macOS

// Basic JUCE namespace and essential classes for compilation
namespace juce {
    // Core types
    class String;
    class File;
    class MemoryBlock;
    class Random;
    class Time;
    
    // Audio basics
    class AudioSampleBuffer;
    class AudioBuffer;
    template<typename T> class AudioBuffer;
    using AudioBufferF = AudioBuffer<float>;
    
    // Audio processing
    class AudioProcessor;
    class AudioProcessorEditor;
    class AudioProcessorParameter;
    class AudioParameterFloat;
    class AudioParameterChoice;
    class AudioParameterBool;
    
    // GUI components
    class Component;
    class Slider;
    class Button;
    class Label;
    class ComboBox;
    class LookAndFeel;
    class Graphics;
    class Colour;
    class Font;
    class Rectangle;
    template<typename T> class Rectangle;
    using Rectanglef = Rectangle<float>;
    using Rectanglei = Rectangle<int>;
    
    // Events
    class MouseEvent;
    class KeyPress;
    class Timer;
    
    // Essential constants and utilities  
    static constexpr float MathConstants_pi = 3.141592653589793f;
    static constexpr float MathConstants_e = 2.718281828459045f;
}

// Plugin-specific defines that would come from CMake
#ifndef JucePlugin_Name
#define JucePlugin_Name "MarsiAutoTune"
#endif

#ifndef JucePlugin_Desc
#define JucePlugin_Desc "Professional AutoTune Plugin by MarsiStudio"
#endif

#ifndef JucePlugin_Manufacturer
#define JucePlugin_Manufacturer "MarsiStudio"
#endif

#ifndef JucePlugin_ManufacturerCode
#define JucePlugin_ManufacturerCode 0x4d727369 // 'Mrsi'
#endif

#ifndef JucePlugin_PluginCode
#define JucePlugin_PluginCode 0x4d724174 // 'MrAt'
#endif

#ifndef JucePlugin_IsSynth
#define JucePlugin_IsSynth 0
#endif

#ifndef JucePlugin_WantsMidiInput
#define JucePlugin_WantsMidiInput 0
#endif

#ifndef JucePlugin_ProducesMidiOutput
#define JucePlugin_ProducesMidiOutput 0
#endif

#ifndef JucePlugin_IsMidiEffect
#define JucePlugin_IsMidiEffect 0
#endif

#ifndef JucePlugin_EditorRequiresKeyboardFocus
#define JucePlugin_EditorRequiresKeyboardFocus 0
#endif

#ifndef JucePlugin_Version
#define JucePlugin_Version 0x10000
#endif

#ifndef JucePlugin_VersionCode
#define JucePlugin_VersionCode JucePlugin_Version
#endif

#ifndef JucePlugin_VersionString
#define JucePlugin_VersionString "1.0.0"
#endif

// Preprocessor definitions for JUCE compatibility
#define JUCE_DECLARE_NON_COPYABLE(className) \
    className (const className&) = delete; \
    className& operator= (const className&) = delete;

#define JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(className) \
    JUCE_DECLARE_NON_COPYABLE(className)

#define JUCE_LEAK_DETECTOR(className)

// Platform detection
#if defined(__APPLE__)
    #define JUCE_MAC 1
#elif defined(_WIN32)
    #define JUCE_WINDOWS 1
#elif defined(__linux__)
    #define JUCE_LINUX 1
#endif

// Binary data declarations for embedded assets
namespace BinaryData
{
    extern const char* logo_svg;
    extern const int logo_svgSize;
    
    extern const char* knob_background_svg;
    extern const int knob_background_svgSize;
    
    extern const char* knob_pointer_svg;
    extern const int knob_pointer_svgSize;
}

// Common utility macros
#define MARSISTUDIO_VERSION_STRING "1.0.0"
#define MARSISTUDIO_BUILD_DATE __DATE__

// Debug logging helper (stub)
#if defined(DEBUG) || defined(_DEBUG)
    #define MARSI_DBG(text) // Debug logging stub
    #define JUCE_DEBUG 1
#else
    #define MARSI_DBG(text)
    #define JUCE_DEBUG 0
#endif

#define MARSI_PROFILE_BLOCK(name) // Performance profiling stub

// Namespace aliases for convenience
using namespace juce;