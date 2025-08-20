#pragma once

// JUCE Library header for MarsiAutoTune
// This provides cross-platform JUCE access

#ifdef __APPLE__
// On macOS, use full JUCE installation
#include <JuceHeader.h>
#else
// On Linux, use downloaded JUCE from /tmp/JUCE
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