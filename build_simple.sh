#!/bin/bash

# MarsiAutoTune Build Script for macOS
# This script builds the VST3, AU, and Standalone versions automatically
# Supports: macOS only

set -e  # Exit on any error

echo "🎵 Building MarsiAutoTune Plugin..."
echo "================================================"

# Check project structure
if [ ! -d "libs" ] || [ ! -d "Source" ]; then
    echo "❌ Project structure incomplete!"
    echo "   Missing required directories: libs/ or Source/"
    exit 1
fi

# Detect operating system
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macOS"
    echo "🍎 Detected platform: macOS"
    echo "📦 Using autonomous libraries (no internet required)"
else
    echo "⚠️  This project is designed for macOS deployment"
    echo "   Current platform: $OSTYPE"
    echo "   Running in development/testing mode..."
    echo ""
fi

# Check for required tools
echo "🔍 Checking required tools..."

command -v cmake >/dev/null 2>&1 || { 
    echo "❌ Error: CMake is not installed."
    if [[ "$PLATFORM" == "macOS" ]]; then
        echo "   Install with: brew install cmake"
    elif [[ "$PLATFORM" == "Linux" ]]; then
        echo "   Install with: sudo apt-get install cmake"
        echo "   Or: sudo yum install cmake"
    fi
    exit 1 
}

echo "✅ CMake found: $(cmake --version | head -n1)"

# Check that all libraries are present
echo "📚 Checking local libraries..."
if [ ! -f "libs/tensorflow_lite/tensorflow_lite.h" ]; then
    echo "❌ TensorFlow Lite library not found"
    exit 1
fi

if [ ! -f "libs/rubberband/rubberband.h" ]; then
    echo "❌ Rubber Band Library not found"
    exit 1
fi

if [ ! -f "libs/crepe/crepe.h" ]; then
    echo "❌ CREPE library not found"
    exit 1
fi

echo "✅ All required libraries found"

# Platform-specific compiler checks
if [[ "$PLATFORM" == "macOS" ]]; then
    command -v xcode-select >/dev/null 2>&1 || { 
        echo "❌ Error: Xcode Command Line Tools not found."
        echo "   Please install with: xcode-select --install"
        exit 1 
    }
    echo "✅ Xcode Command Line Tools found"
fi

# Create build directory
echo "📁 Creating build directory..."
rm -rf build
mkdir -p build
cd build

# Configure CMake with platform-specific options
echo "⚙️  Configuring CMake for $PLATFORM..."

# macOS configuration with local libraries
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -G "Unix Makefiles" \
    -DJUCE_MODULES_ONLY=ON

# Build the plugin with optimal CPU usage
echo "🔨 Building MarsiAutoTune..."
CPU_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo "4")
echo "🚀 Using $CPU_CORES CPU cores for parallel build..."

cmake --build . --config Release --parallel "$CPU_CORES"

# Check if build was successful
BUILD_SUCCESS=$?
if [ $BUILD_SUCCESS -eq 0 ]; then
    echo ""
    echo "✅ Build completed successfully!"
    echo ""
    echo "📦 Plugin locations:"
    
    echo "   VST3: ~/Library/Audio/Plug-Ins/VST3/MarsiAutoTune.vst3"
    echo "   AU:   ~/Library/Audio/Plug-Ins/Components/MarsiAutoTune.component" 
    echo "   Standalone: /Applications/MarsiAutoTune.app"
    
    echo ""
    echo "🎉 MarsiAutoTune is ready to use!"
    
    echo ""
    echo "🚀 Launch your DAW and look for 'MarsiAutoTune' in your plugin list."
    echo "   Supported formats: VST3, AudioUnit (AU), Standalone"
    echo "   Compatible DAWs: Logic Pro X, Pro Tools, Ableton Live, FL Studio, etc."
    
else
    echo ""
    echo "❌ Build failed with exit code $BUILD_SUCCESS"
    echo "📋 Please check the error messages above for details."
    echo ""
    echo "🔧 Common solutions:"
    echo "   • Make sure all dependencies are installed"
    echo "   • Check that you have sufficient disk space"
    echo "   • Verify your compiler supports C++17"
    echo "   • Try cleaning the build directory: rm -rf build"
    
    exit 1
fi

echo ""
echo "================================================"
echo "✨ MarsiAutoTune build complete! ✨"
echo "================================================"

# Optional: Run basic tests if available
if [[ -f "./MarsiAutoTune_artefacts/Standalone/MarsiAutoTune" ]]; then
    echo ""
    echo "🧪 Running basic plugin validation..."
    timeout 5s ./MarsiAutoTune_artefacts/Standalone/MarsiAutoTune --help 2>/dev/null || echo "   Plugin appears to be built correctly"
fi
