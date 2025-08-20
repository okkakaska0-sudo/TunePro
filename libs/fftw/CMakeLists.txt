# FFTW3 локальная библиотека для MarsiAutoTune

set(FFTW_SOURCES
    fftw3.cpp
    fftw3.h
)

add_library(fftw_static STATIC ${FFTW_SOURCES})

target_include_directories(fftw_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(fftw_static PUBLIC cxx_std_17)

# Добавляем оптимизации для FFT
target_compile_options(fftw_static PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-O3 -ffast-math -funroll-loops -msse2>
    $<$<CXX_COMPILER_ID:MSVC>:/O2 /arch:SSE2>
)

target_compile_definitions(fftw_static PRIVATE
    FFTW_ENABLE_FLOAT=1
    FFTW_DLL=1
)

# macOS специфические настройки
if(APPLE)
    target_compile_definitions(fftw_static PRIVATE
        FFTW_MACOS=1
    )
    
    # Используем Accelerate framework для векторных операций
    target_link_libraries(fftw_static PUBLIC "-framework Accelerate")
endif()