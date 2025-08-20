# Rubber Band Library локальная реализация для MarsiAutoTune

set(RUBBERBAND_SOURCES
    RubberBandStretcher.cpp
    rubberband.h
    RubberBandStretcher.h
)

add_library(rubberband_static STATIC ${RUBBERBAND_SOURCES})

target_include_directories(rubberband_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rubberband_static PUBLIC cxx_std_17)

# Добавляем оптимизации для аудио обработки
target_compile_options(rubberband_static PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-O3 -ffast-math -funroll-loops>
    $<$<CXX_COMPILER_ID:MSVC>:/O2>
)

target_compile_definitions(rubberband_static PRIVATE
    RUBBERBAND_STATIC=1
    USE_PTHREADS=1
)

# macOS специфические настройки
if(APPLE)
    target_compile_definitions(rubberband_static PRIVATE
        RUBBERBAND_MACOS=1
        USE_PTHREADS=1
    )
    
    # Используем Accelerate framework и Core Audio
    target_link_libraries(rubberband_static PUBLIC 
        "-framework Accelerate"
        "-framework CoreAudio"
    )
endif()