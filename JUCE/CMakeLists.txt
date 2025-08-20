# JUCE Framework stub - minimal CMakeLists.txt
# This will be replaced by actual JUCE when built on macOS
cmake_minimum_required(VERSION 3.20)

project(JUCE)

# Create dummy targets for development
add_library(juce_audio_basics INTERFACE)
add_library(juce_audio_devices INTERFACE) 
add_library(juce_audio_formats INTERFACE)
add_library(juce_audio_processors INTERFACE)
add_library(juce_audio_utils INTERFACE)
add_library(juce_core INTERFACE)
add_library(juce_data_structures INTERFACE)
add_library(juce_events INTERFACE)
add_library(juce_dsp INTERFACE)
add_library(juce_gui_basics INTERFACE)
add_library(juce_gui_extra INTERFACE)
add_library(juce_graphics INTERFACE)
add_library(juce_cryptography INTERFACE)

# Create aliases matching JUCE naming
add_library(juce::juce_audio_basics ALIAS juce_audio_basics)
add_library(juce::juce_audio_devices ALIAS juce_audio_devices)
add_library(juce::juce_audio_formats ALIAS juce_audio_formats)
add_library(juce::juce_audio_processors ALIAS juce_audio_processors)
add_library(juce::juce_audio_utils ALIAS juce_audio_utils)
add_library(juce::juce_core ALIAS juce_core)
add_library(juce::juce_data_structures ALIAS juce_data_structures)
add_library(juce::juce_events ALIAS juce_events)
add_library(juce::juce_dsp ALIAS juce_dsp)
add_library(juce::juce_gui_basics ALIAS juce_gui_basics)
add_library(juce::juce_gui_extra ALIAS juce_gui_extra)
add_library(juce::juce_graphics ALIAS juce_graphics)
add_library(juce::juce_cryptography ALIAS juce_cryptography)

# Create dummy recommended flags
add_library(juce_recommended_config_flags INTERFACE)
add_library(juce_recommended_lto_flags INTERFACE)
add_library(juce_recommended_warning_flags INTERFACE)

add_library(juce::juce_recommended_config_flags ALIAS juce_recommended_config_flags)
add_library(juce::juce_recommended_lto_flags ALIAS juce_recommended_lto_flags)
add_library(juce::juce_recommended_warning_flags ALIAS juce_recommended_warning_flags)

# Dummy juce_add_plugin function
function(juce_add_plugin target)
    message(STATUS "Creating dummy plugin target: ${target}")
    add_library(${target} SHARED ${ARGN})
endfunction()

message(STATUS "JUCE stub loaded - this will download actual JUCE on macOS build")