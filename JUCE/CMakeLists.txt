# JUCE Framework integration
cmake_minimum_required(VERSION 3.20)

project(JUCE_Integration)

# Определяем платформу
if(APPLE)
    # На macOS используем системный JUCE через FetchContent
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
    message(STATUS "JUCE loaded from GitHub for macOS build")
else()
    # На Linux используем загруженный JUCE 
    if(EXISTS "/tmp/JUCE/CMakeLists.txt")
        message(STATUS "Loading JUCE from /tmp/JUCE")
        add_subdirectory(/tmp/JUCE JUCE_BUILD EXCLUDE_FROM_ALL)
        message(STATUS "JUCE loaded successfully for development")
    else()
        message(FATAL_ERROR "JUCE not found in /tmp/JUCE. Please run: git clone https://github.com/juce-framework/JUCE.git /tmp/JUCE")
    endif()
endif()

# Создаем wrapper функцию для обеих платформ
function(marsi_add_plugin target)
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
    )
    
    # Исключаем backup и stub файлы
    list(FILTER PLUGIN_SOURCES EXCLUDE REGEX ".*_backup\\.cpp$")
    list(FILTER PLUGIN_SOURCES EXCLUDE REGEX ".*_linux_stub\\.cpp$")
    
    # Добавляем локальные библиотеки только для macOS
    if(APPLE)
        list(APPEND PLUGIN_SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/libs/crepe/crepe.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/libs/tensorflow_lite/tensorflow_lite.cpp"  
            "${CMAKE_CURRENT_SOURCE_DIR}/libs/rubberband/RubberBandStretcher.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/libs/fftw/fftw3.cpp"
        )
    endif()
    
    message(STATUS "Found ${list_length} source files for ${target}")
    
    # Создаем цель с помощью стандартного JUCE
    if(TARGET juce::juce_audio_basics AND COMMAND juce_add_plugin)
        # Используем оригинальную JUCE функцию если доступна
        cmake_language(CALL juce_add_plugin ${target} ${ARGN})
        set(JUCE_TARGET_CREATED TRUE)
    else()
        # Создаем цель вручную для stub режима
        add_library(${target} SHARED)
        set(JUCE_TARGET_CREATED FALSE)
    endif()
    
    # Добавляем исходники к цели
    target_sources(${target} PRIVATE ${PLUGIN_SOURCES})
    
    # Добавляем include directories для нахождения JuceHeader.h
    target_include_directories(${target} PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    )
    
    # Настройки компиляции только если JUCE не создал цель сам
    if(NOT JUCE_TARGET_CREATED)
        target_compile_definitions(${target} 
            PUBLIC
                JUCE_WEB_BROWSER=0
                JUCE_USE_CURL=0
                JUCE_DISABLE_JUCE_VERSION_PRINTING=1
            PRIVATE
                JUCE_DISPLAY_SPLASH_SCREEN=0
                JUCE_REPORT_APP_USAGE=0
                JUCE_STRICT_REFCOUNTEDPOINTER=1
                JUCE_VST3_CAN_REPLACE_VST2=0)

        if(TARGET juce::juce_audio_basics)
            target_link_libraries(${target}
                PRIVATE
                    juce::juce_audio_basics
                    juce::juce_audio_devices
                    juce::juce_audio_formats
                    juce::juce_audio_processors
                    juce::juce_audio_utils
                    juce::juce_core
                    juce::juce_data_structures
                    juce::juce_dsp
                    juce::juce_events
                    juce::juce_graphics
                    juce::juce_gui_basics
                    juce::juce_gui_extra
                PUBLIC
                    juce::juce_recommended_config_flags
                    juce::juce_recommended_lto_flags
                    juce::juce_recommended_warning_flags)
        endif()
    endif()
    
    message(STATUS "Plugin ${target} configured successfully")
endfunction()