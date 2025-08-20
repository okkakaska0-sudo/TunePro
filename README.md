# MarsiAutoTune - Professional VST3/AU AutoTune Plugin

## 🎵 Overview

MarsiAutoTune is a professional-grade pitch correction plugin for macOS, featuring three distinct processing modes and a beautiful vintage skeuomorphic interface. Built with JUCE framework for maximum compatibility and performance.

## ✨ Features

### Core Processing Modes
- **Classic Mode**: Natural, smooth vocal correction perfect for pop and mainstream music
- **Hard Mode**: Aggressive T-Pain style effect with immediate pitch snapping
- **AI Mode**: Intelligent pitch correction with advanced formant preservation

### Key Parameters
- **Speed (0-100)**: Controls the correction response time
- **Amount (0-100)**: Intensity of pitch correction
- **Key Selection**: Choose from all 12 chromatic keys (C, C#, D, D#, E, F, F#, G, G#, A, A#, B)
- **Scale Modes**: Major, Minor, Chromatic scales
- **Preset System**: Save and load custom settings

### Professional Features
- Real-time pitch detection and correction
- Formant preservation in all modes
- Low-latency processing optimized for live performance
- Professional preset collection
- Vintage rack-style interface design

## 🛠️ System Requirements

### macOS Requirements
- macOS 10.13 (High Sierra) or later
- Intel or Apple Silicon Mac
- Audio Units (AU) or VST3 compatible DAW

### Supported DAWs
- Logic Pro
- Pro Tools
- Ableton Live
- Reaper
- GarageBand
- And many more AU/VST3 compatible hosts

## 📦 Installation

### Quick Install
1. Download the latest release
2. Run the installer package
3. Launch your DAW and scan for new plugins
4. Look for "MarsiAutoTune" in your plugin list

### Manual Build (Advanced Users)
```bash
# Clone the repository
git clone https://github.com/marsistudio/marsi-autotune.git
cd marsi-autotune

# Run the build script
chmod +x build_simple.sh
./build_simple.sh
