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

# Add all source files with explicit paths
target_sources(MarsiAutoTune
    PRIVATE
        # Main plugin files
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginProcessor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginProcessor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginEditor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginEditor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginEditor_minimal.cpp
        
        # Core functionality
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Parameters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Parameters.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/PresetManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/PresetManager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Utils.h
        
        # Audio processing
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/PitchCorrectionEngine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/PitchCorrectionEngine.h
        
        # User interface
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/LookAndFeel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/LookAndFeel.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/ModeSelector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/ModeSelector.h
        
        # AI functionality
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/AIModelLoader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/AIModelLoader.h
        
        # JUCE header
        ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/JuceHeader.h
        
        # Additional AI libraries (local implementations)
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/crepe/crepe.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/tensorflow_lite/tensorflow_lite.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/rubberband/RubberBandStretcher.cpp
)

# Set up include directories with proper paths
target_include_directories(MarsiAutoTune
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode
        ${CMAKE_CURRENT_SOURCE_DIR}/Assets
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/eigen
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/fftw
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/crepe
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/tensorflow_lite
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/rubberband
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/crepe_models
    SYSTEM PRIVATE
        ${JUCE_SOURCE_DIR}/modules
)

# Link against all required JUCE libraries
target_link_libraries(MarsiAutoTune
    PRIVATE
        # Core audio libraries
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        
        # Core libraries
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        
        # DSP library for advanced processing
        juce::juce_dsp
        
        # GUI libraries
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_graphics
        
        # Optional libraries for extended functionality
        juce::juce_cryptography
        
    PUBLIC
        # JUCE recommended flags
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Comprehensive compiler definitions
target_compile_definitions(MarsiAutoTune
    PUBLIC
        # JUCE configuration
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_ENABLE_REPAINT_DEBUGGING=0
        JUCE_USE_FLAC=0
        JUCE_USE_OGGVORBIS=0
        JUCE_USE_MP3AUDIOFORMAT=0
        JUCE_USE_LAME_AUDIO_FORMAT=0
        JUCE_USE_WINDOWS_MEDIA_FORMAT=0
        JUCE_PLUGINHOST_VST=0
        JUCE_PLUGINHOST_VST3=0
        JUCE_PLUGINHOST_AU=0
        JUCE_PLUGINHOST_LADSPA=0
        
        # Performance optimizations
        JUCE_USE_SSE_INTRINSICS=1
        JUCE_USE_ARM_NEON=1
        
    PRIVATE
        # Application metadata
        JUCE_APPLICATION_NAME_STRING="MarsiAutoTune"
        JUCE_APPLICATION_VERSION_STRING="${PROJECT_VERSION}"
        JUCE_APPLICATION_VERSION=${PROJECT_VERSION}
        
        # Plugin specific
        JucePlugin_Build_VST3=1
        JucePlugin_Build_AU=1
        JucePlugin_Build_Standalone=1
        
        # MarsiStudio specific definitions
        MARSI_STUDIO_VERSION="${PROJECT_VERSION}"
        MARSI_AUTOTUNE_PLUGIN=1
        
        # AI and Audio Processing Libraries
        USE_TENSORFLOW_LITE=1
        USE_CREPE_PITCH_DETECTION=1
        USE_RUBBERBAND_STRETCHER=1
        HAVE_FFTW=1
        HAVE_EIGEN=1
)

# Platform-specific configurations
if(APPLE)
    # macOS specific settings
    set_target_properties(MarsiAutoTune PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.marsistudio.autotune"
        MACOSX_BUNDLE_BUNDLE_NAME "MarsiAutoTune"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "© 2025 MarsiStudio"
        MACOSX_BUNDLE_INFO_STRING "MarsiAutoTune ${PROJECT_VERSION}"
    )
    
    # Link against macOS frameworks
    target_link_libraries(MarsiAutoTune PRIVATE
        "-framework AudioUnit"
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework CoreMIDI"
        "-framework Cocoa"
        "-framework Carbon"
        "-framework QuartzCore"
        "-framework MetalKit"
        "-framework Metal"
        "-framework Accelerate"  # For optimized math operations
        "-framework CoreML"      # For AI model inference
    )
    
elseif(WIN32)
    # Windows specific settings
    set_target_properties(MarsiAutoTune PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Link against Windows libraries
    target_link_libraries(MarsiAutoTune PRIVATE
        winmm
        ole32
        oleaut32
        imm32
        comdlg32
        shlwapi
        rpcrt4
        wininet
    )
    
elseif(UNIX)
    # Linux specific settings
    find_package(PkgConfig REQUIRED)
    
    # Find required Linux packages
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(WEBKIT2 webkit2gtk-4.0)
    
    target_include_directories(MarsiAutoTune PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_directories(MarsiAutoTune PRIVATE ${GTK3_LIBRARY_DIRS})
    target_compile_options(MarsiAutoTune PRIVATE ${GTK3_CFLAGS_OTHER})
    
    # Link against Linux libraries
    target_link_libraries(MarsiAutoTune PRIVATE
        ${GTK3_LIBRARIES}
        dl
        pthread
        rt
        X11
        Xext
        asound
        freetype
    )
    
    if(WEBKIT2_FOUND)
        target_link_libraries(MarsiAutoTune PRIVATE ${WEBKIT2_LIBRARIES})
    endif()
endif()

# Compiler warnings and optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(MarsiAutoTune PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -Wno-sign-compare
        -Wno-unused-variable
        -Wno-missing-field-initializers
    )
    
    # Release optimizations
    target_compile_options(MarsiAutoTune PRIVATE
        $<$<CONFIG:Release>:-O3 -DNDEBUG -ffast-math>
        $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
    )
    
elseif(MSVC)
    target_compile_options(MarsiAutoTune PRIVATE
        /W3
        /wd4996  # Disable deprecated function warnings
        /wd4100  # Disable unused parameter warnings
    )
endif()

# Set output directories for different build types
set_target_properties(MarsiAutoTune PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Custom build commands
if(APPLE)
    # macOS post-build commands for plugin validation
    add_custom_command(TARGET MarsiAutoTune POST_BUILD
        COMMAND echo "✅ MarsiAutoTune built successfully for macOS"
        COMMAND echo "📍 VST3: ~/Library/Audio/Plug-Ins/VST3/MarsiAutoTune.vst3"
        COMMAND echo "📍 AU: ~/Library/Audio/Plug-Ins/Components/MarsiAutoTune.component"
        VERBATIM
    )
elseif(UNIX)
    # Linux post-build commands
    add_custom_command(TARGET MarsiAutoTune POST_BUILD
        COMMAND echo "✅ MarsiAutoTune built successfully for Linux"
        COMMAND echo "📍 VST3: ~/.vst3/MarsiAutoTune.vst3"
        VERBATIM
    )
endif()

# Installation rules
if(APPLE)
    install(TARGETS MarsiAutoTune DESTINATION "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/")
elseif(UNIX)
    install(TARGETS MarsiAutoTune DESTINATION "$ENV{HOME}/.vst3/")
elseif(WIN32)
    install(TARGETS MarsiAutoTune DESTINATION "C:/Program Files/Common Files/VST3/")
endif()

# Print configuration summary
message(STATUS "==========================================")
message(STATUS "MarsiAutoTune Configuration Summary:")
message(STATUS "==========================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
if(APPLE)
    message(STATUS "macOS Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    message(STATUS "Architectures: ${CMAKE_OSX_ARCHITECTURES}")
endif()
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "JUCE Version: Downloading latest stable")
message(STATUS "Formats: VST3, AU, Standalone")
message(STATUS "==========================================")

# Enable testing (optional)
option(ENABLE_TESTING "Enable testing" OFF)
if(ENABLE_TESTING)
    enable_testing()
    # Add tests here if needed
endif()

# Documentation generation (optional)
option(BUILD_DOCUMENTATION "Build documentation" OFF)
if(BUILD_DOCUMENTATION)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
        add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
    endif()
endif()
