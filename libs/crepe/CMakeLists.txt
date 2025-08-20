# CREPE локальная библиотека для MarsiAutoTune

set(CREPE_SOURCES
    crepe.cpp
    crepe.h
)

add_library(crepe_static STATIC ${CREPE_SOURCES})

target_include_directories(crepe_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(crepe_static PUBLIC cxx_std_17)

# Добавляем оптимизации
target_compile_options(crepe_static PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-O3 -ffast-math -funroll-loops>
    $<$<CXX_COMPILER_ID:MSVC>:/O2>
)

# macOS специфические настройки
if(APPLE)
    target_compile_definitions(crepe_static PRIVATE
        CREPE_MACOS=1
    )
    
    # Используем Accelerate framework
    target_link_libraries(crepe_static PUBLIC "-framework Accelerate")
endif()