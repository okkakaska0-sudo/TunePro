cmake_minimum_required(VERSION 3.20)

# Define the project with detailed information
project(MarsiAutoTune 
    VERSION 1.0.0
    DESCRIPTION "Professional Auto-Tune VST3/AU Plugin by MarsiStudio"
    LANGUAGES CXX C
)

# Set C++ standard with specific features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose build type" FORCE)
endif()

# Cross-platform compatibility settings
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment version" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for macOS" FORCE)
elseif(WIN32)
    set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "Windows target version" FORCE)
endif()

# Compiler-specific settings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -DDEBUG")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /DDEBUG")
endif()

# Include modern CMake modules
include(GNUInstallDirs)

# Setup JUCE Framework
if(APPLE AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules")
    message(STATUS "Downloading JUCE framework for macOS build...")
    find_package(Git QUIET)
    if(Git_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} clone --branch 7.0.12 --depth 1 https://github.com/juce-framework/JUCE.git
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_QUIET
            ERROR_QUIET
        )
    endif()
endif()

# Configure JUCE options
option(JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

# Add JUCE
set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
if(EXISTS "${JUCE_DIR}/modules")
    add_subdirectory(${JUCE_DIR})
    message(STATUS "Using full JUCE framework")
else()
    add_subdirectory(${JUCE_DIR})
    message(STATUS "Using JUCE stub - will work fully on macOS")
endif()

# Set JUCE compile definitions globally
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

# Define plugin with comprehensive configuration
juce_add_plugin(MarsiAutoTune
    VERSION ${PROJECT_VERSION}
    COMPANY_NAME "MarsiStudio"
    COMPANY_WEBSITE "https://marsistudio.com"
    COMPANY_EMAIL "support@marsistudio.com"
    MANUFACTURER_CODE "Mrsi"
    PLUGIN_CODE "MrAt"
    
    # Plugin type and MIDI settings
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    
    # Formats to build
    FORMATS AU VST3 Standalone
    
    # Product naming
    PRODUCT_NAME "MarsiAutoTune"
    PLUGIN_NAME "MarsiAutoTune"
    
    # Bundle configuration
    BUNDLE_ID "com.marsistudio.autotune"
    
    # Build settings
    COPY_PLUGIN_AFTER_BUILD TRUE
    
    # Permissions
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "This plugin needs access to your microphone for real-time pitch correction."
    
    # Icons (if available)
    # ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Assets/icon_big.png"
    # ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Assets/icon_small.png"
    
    # VST3 specific
    VST3_CATEGORIES "Fx" "Pitch Shift"
    
    # AU specific  
    AU_MAIN_TYPE "kAudioUnitType_Effect"
    AU_SANDBOX_SAFE TRUE
)

# Все остальные настройки делаются внутри juce_add_plugin функции
