/*
  JUCE Library Header File
  
  This file includes all the JUCE modules used by MarsiAutoTune plugin.
  It provides a single include point for all JUCE functionality.
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

// Plugin-specific defines
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

// Namespace aliases for convenience
using namespace juce;
using namespace juce::dsp;

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

// Debug logging helper
#if JUCE_DEBUG
    #define MARSI_DBG(text) DBG("MarsiAutoTune: " << text)
#else
    #define MARSI_DBG(text)
#endif

// Performance profiling helper
#if JUCE_DEBUG
    #define MARSI_PROFILE_BLOCK(name) \
        ScopedProfiler profiler(name)
    
    class ScopedProfiler
    {
    public:
        ScopedProfiler(const String& n) : name(n), startTime(Time::getHighResolutionTicks()) {}
        ~ScopedProfiler() 
        { 
            auto elapsed = Time::getHighResolutionTicksPerSecond() > 0 ?
                (Time::getHighResolutionTicks() - startTime) / Time::getHighResolutionTicksPerSecond() * 1000.0 : 0.0;
            MARSI_DBG(name << " took " << elapsed << "ms"); 
        }
    private:
        String name;
        int64 startTime;
    };
#else
    #define MARSI_PROFILE_BLOCK(name)
#endif
