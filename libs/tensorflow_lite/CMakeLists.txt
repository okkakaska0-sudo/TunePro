# TensorFlow Lite локальная библиотека для MarsiAutoTune
# Упрощенная реализация для автономной работы

set(TENSORFLOW_LITE_SOURCES
    tensorflow_lite.cpp
    tensorflow_lite.h
)

add_library(tensorflow_lite STATIC ${TENSORFLOW_LITE_SOURCES})

target_include_directories(tensorflow_lite PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Компилируем с C++17 для совместимости
target_compile_features(tensorflow_lite PUBLIC cxx_std_17)

# Добавляем флаги оптимизации
target_compile_options(tensorflow_lite PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-O3 -ffast-math>
    $<$<CXX_COMPILER_ID:MSVC>:/O2>
)

# macOS специфические настройки
if(APPLE)
    target_compile_definitions(tensorflow_lite PRIVATE
        TENSORFLOW_LITE_MACOS=1
    )
    
    # Используем Accelerate framework для векторных операций
    target_link_libraries(tensorflow_lite PUBLIC "-framework Accelerate")
endif()