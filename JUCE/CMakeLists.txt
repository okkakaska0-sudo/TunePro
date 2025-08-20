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

# Реализация juce_add_plugin для работы с уже существующим target
function(juce_add_plugin target)
    cmake_parse_arguments(PLUGIN 
        "IS_SYNTH;NEEDS_MIDI_INPUT;NEEDS_MIDI_OUTPUT;IS_MIDI_EFFECT;EDITOR_WANTS_KEYBOARD_FOCUS;COPY_PLUGIN_AFTER_BUILD;MICROPHONE_PERMISSION_ENABLED"
        "VERSION;COMPANY_NAME;COMPANY_WEBSITE;COMPANY_EMAIL;MANUFACTURER_CODE;PLUGIN_CODE;PRODUCT_NAME;PLUGIN_NAME;BUNDLE_ID;MICROPHONE_PERMISSION_TEXT;ICON_BIG;ICON_SMALL"
        "FORMATS;VST3_CATEGORIES;AU_MAIN_TYPE;PLUGIN_CHARACTERISTICS_VALUE"
        ${ARGN}
    )
    
    message(STATUS "Configuring plugin: ${target}")
    message(STATUS "Version: ${PLUGIN_VERSION}")
    message(STATUS "Formats: ${PLUGIN_FORMATS}")
    message(STATUS "Company: ${PLUGIN_COMPANY_NAME}")
    
    # Собираем все исходные файлы из Source/
    file(GLOB_RECURSE PLUGIN_SOURCES 
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/crepe/crepe.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/tensorflow_lite/tensorflow_lite.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/rubberband/RubberBandStretcher.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/fftw/fftw3.cpp"
    )
    
    # Создаем новый shared library target
    add_library(${target} SHARED ${PLUGIN_SOURCES})
    list(LENGTH PLUGIN_SOURCES NUM_SOURCES)
    message(STATUS "Created plugin target: ${target} with ${NUM_SOURCES} source files")
    
    # Настройки компиляции
    target_compile_features(${target} PRIVATE cxx_std_17)
    
    # Включаем все необходимые директории
    target_include_directories(${target} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
        "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode"
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/crepe"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/tensorflow_lite"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/rubberband"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/fftw"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/eigen"
    )
    
    # Linking JUCE libraries
    target_link_libraries(${target} PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_graphics
        juce::juce_cryptography
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )
    
    # macOS специфические настройки
    if(APPLE)
        # Линкуем системные фреймворки
        target_link_libraries(${target} PRIVATE
            "-framework CoreFoundation"
            "-framework CoreAudio" 
            "-framework CoreMIDI"
            "-framework AudioUnit"
            "-framework AudioToolbox"
            "-framework Accelerate"
            "-framework Cocoa"
            "-framework Carbon"
            "-framework QuartzCore"
            "-framework IOKit"
            "-framework DiscRecording"
            "-framework WebKit"
        )
        
        # Установка архитектур
        set_target_properties(${target} PROPERTIES
            OSX_ARCHITECTURES "x86_64;arm64"
            MACOSX_DEPLOYMENT_TARGET "10.13"
        )
        
        # Настройки для разных форматов
        foreach(format ${PLUGIN_FORMATS})
            if(format STREQUAL "VST3")
                # VST3 специфические настройки
                set_target_properties(${target} PROPERTIES
                    BUNDLE TRUE
                    BUNDLE_EXTENSION "vst3"
                    OUTPUT_NAME "${PLUGIN_PRODUCT_NAME}"
                    XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst3"
                )
                
                # Установка VST3 в системную директорию
                install(TARGETS ${target}
                    BUNDLE DESTINATION "$ENV{HOME}/Library/Audio/Plug-Ins/VST3"
                )
                
            elseif(format STREQUAL "AU")
                # AudioUnit специфические настройки
                set_target_properties(${target} PROPERTIES
                    BUNDLE TRUE
                    BUNDLE_EXTENSION "component"
                    OUTPUT_NAME "${PLUGIN_PRODUCT_NAME}"
                    XCODE_ATTRIBUTE_WRAPPER_EXTENSION "component"
                )
                
                # Установка AU в системную директорию
                install(TARGETS ${target}
                    BUNDLE DESTINATION "$ENV{HOME}/Library/Audio/Plug-Ins/Components"
                )
                
            elseif(format STREQUAL "Standalone")
                # Standalone приложение
                set_target_properties(${target} PROPERTIES
                    BUNDLE TRUE
                    BUNDLE_EXTENSION "app"
                    OUTPUT_NAME "${PLUGIN_PRODUCT_NAME}"
                    MACOSX_BUNDLE TRUE
                    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
                )
                
                # Установка в Applications
                install(TARGETS ${target}
                    BUNDLE DESTINATION "/Applications"
                )
            endif()
        endforeach()
        
        # Компилятор флаги для оптимизации
        target_compile_options(${target} PRIVATE
            -O3
            -ffast-math
            -funroll-loops
            -fomit-frame-pointer
        )
        
        # Линкер флаги
        target_link_options(${target} PRIVATE
            -Wl,-dead_strip
            -Wl,-x
        )
    endif()
    
    # Полные определения препроцессора для JUCE и плагина
    target_compile_definitions(${target} PRIVATE
        # JUCE основные настройки
        JUCE_STANDALONE_APPLICATION=$<IF:$<IN_LIST:Standalone,${PLUGIN_FORMATS}>,1,0>
        JUCE_USE_DARK_APPEARANCE=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_USE_SSE_INTRINSICS=1
        JUCE_USE_ARM_NEON=1
        
        # Plugin идентификация
        JucePlugin_Build_VST=$<IF:$<IN_LIST:VST3,${PLUGIN_FORMATS}>,1,0>
        JucePlugin_Build_VST3=$<IF:$<IN_LIST:VST3,${PLUGIN_FORMATS}>,1,0>
        JucePlugin_Build_AU=$<IF:$<IN_LIST:AU,${PLUGIN_FORMATS}>,1,0>
        JucePlugin_Build_Standalone=$<IF:$<IN_LIST:Standalone,${PLUGIN_FORMATS}>,1,0>
        JucePlugin_Name="${PLUGIN_PRODUCT_NAME}"
        JucePlugin_Desc="${PLUGIN_PRODUCT_NAME} - Professional AutoTune Plugin"
        JucePlugin_Manufacturer="${PLUGIN_COMPANY_NAME}"
        JucePlugin_ManufacturerWebsite="${PLUGIN_COMPANY_WEBSITE}"
        JucePlugin_ManufacturerEmail="${PLUGIN_COMPANY_EMAIL}"
        JucePlugin_ManufacturerCode='${PLUGIN_MANUFACTURER_CODE}'
        JucePlugin_PluginCode='${PLUGIN_PLUGIN_CODE}'
        JucePlugin_IsSynth=$<BOOL:${PLUGIN_IS_SYNTH}>
        JucePlugin_WantsMidiInput=$<BOOL:${PLUGIN_NEEDS_MIDI_INPUT}>
        JucePlugin_ProducesMidiOutput=$<BOOL:${PLUGIN_NEEDS_MIDI_OUTPUT}>
        JucePlugin_IsMidiEffect=$<BOOL:${PLUGIN_IS_MIDI_EFFECT}>
        JucePlugin_EditorRequiresKeyboardFocus=$<BOOL:${PLUGIN_EDITOR_WANTS_KEYBOARD_FOCUS}>
        JucePlugin_Version=${PLUGIN_VERSION}
        JucePlugin_VersionCode=0x010000
        JucePlugin_VersionString="${PLUGIN_VERSION}"
        
        # MarsiStudio специфические определения
        MARSI_STUDIO_VERSION="${PLUGIN_VERSION}"
        MARSI_AUTOTUNE_PLUGIN=1
        USE_TENSORFLOW_LITE=1
        USE_CREPE_PITCH_DETECTION=1
        USE_RUBBERBAND_STRETCHER=1
        HAVE_FFTW=1
        HAVE_EIGEN=1
        
        # Приложение метаданные
        JUCE_APPLICATION_NAME_STRING="${PLUGIN_PRODUCT_NAME}"
        JUCE_APPLICATION_VERSION_STRING="${PLUGIN_VERSION}"
        
        # Build type definitions
        $<$<CONFIG:Debug>:DEBUG=1 _DEBUG=1>
        $<$<CONFIG:Release>:NDEBUG=1>
    )
    
    message(STATUS "Plugin ${target} configured successfully")
endfunction()

message(STATUS "JUCE stub loaded - this will download actual JUCE on macOS build")