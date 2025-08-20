#pragma once

// JUCE Library header for MarsiAutoTune
// This provides cross-platform JUCE access

// Define plugin metadata that JUCE expects
#define JucePlugin_Name                   "MarsiAutoTune"
#define JucePlugin_Desc                   "Professional Auto-Tune Plugin"  
#define JucePlugin_Manufacturer           "MarsiStudio"
#define JucePlugin_ManufacturerWebsite    "https://marsistudio.com"
#define JucePlugin_ManufacturerEmail      "support@marsistudio.com"
#define JucePlugin_ManufacturerCode       0x4d617273  // 'Mars'
#define JucePlugin_PluginCode             0x4d415454  // 'MATT'
#define JucePlugin_IsSynth                0
#define JucePlugin_WantsMidiInput         1
#define JucePlugin_ProducesMidiOutput     0
#define JucePlugin_IsMidiEffect           0
#define JucePlugin_EditorRequiresKeyboardFocus  1
#define JucePlugin_Version                1.0.0
#define JucePlugin_VersionCode            0x10000
#define JucePlugin_VersionString          "1.0.0"
#define JucePlugin_VSTUniqueID            JucePlugin_PluginCode
#define JucePlugin_VSTCategory            kPlugCategEffect
#define JucePlugin_Vst3Category           "Fx"
#define JucePlugin_AUMainType             'aufx'
#define JucePlugin_AUSubType              JucePlugin_PluginCode
#define JucePlugin_AUExportPrefix         MarsiAutoTuneAU
#define JucePlugin_AUExportPrefixQuoted   "MarsiAutoTuneAU"
#define JucePlugin_AUManufacturerCode     JucePlugin_ManufacturerCode
#define JucePlugin_CFBundleIdentifier     com.marsistudio.MarsiAutoTune
#define JucePlugin_AAXIdentifier          com.marsistudio.MarsiAutoTune
#define JucePlugin_AAXManufacturerCode    JucePlugin_ManufacturerCode
#define JucePlugin_AAXProductId           JucePlugin_PluginCode
#define JucePlugin_AAXCategory            2048  // ePlugInCategory_Effect
#define JucePlugin_RTASCategory           2048

// Both platforms use /tmp/JUCE for consistency  
#define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 1

#include "/tmp/JUCE/modules/juce_core/juce_core.h"
#include "/tmp/JUCE/modules/juce_audio_basics/juce_audio_basics.h"
#include "/tmp/JUCE/modules/juce_audio_devices/juce_audio_devices.h"
#include "/tmp/JUCE/modules/juce_audio_formats/juce_audio_formats.h"
#include "/tmp/JUCE/modules/juce_audio_processors/juce_audio_processors.h"
#include "/tmp/JUCE/modules/juce_audio_utils/juce_audio_utils.h"
#include "/tmp/JUCE/modules/juce_data_structures/juce_data_structures.h"
#include "/tmp/JUCE/modules/juce_dsp/juce_dsp.h"
#include "/tmp/JUCE/modules/juce_events/juce_events.h"
#include "/tmp/JUCE/modules/juce_graphics/juce_graphics.h"
#include "/tmp/JUCE/modules/juce_gui_basics/juce_gui_basics.h"
#include "/tmp/JUCE/modules/juce_gui_extra/juce_gui_extra.h"

// Use the JUCE namespace 
using namespace juce;

// Linux-specific stubs for development only
#ifndef __APPLE__
class MarsiLogger {
public:
    static void writeToLog(const std::string& message) {
        std::cout << "[MarsiLogger] " << message << std::endl;
    }
    
    static void log(const std::string& message) {
        writeToLog(message);
    }
};
#endif // __APPLE__

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