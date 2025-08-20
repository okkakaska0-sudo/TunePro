#pragma once

#include "JuceHeader.h"
#include <vector>

class Parameters
{
public:
    // Parameter IDs
    static const String SPEED_ID;
    static const String AMOUNT_ID;
    static const String MODE_ID;
    static const String KEY_ID;
    static const String SCALE_ID;
    
    // Parameter ranges and defaults
    static constexpr float SPEED_MIN = 0.0f;
    static constexpr float SPEED_MAX = 100.0f;
    static constexpr float SPEED_DEFAULT = 50.0f;
    static constexpr float SPEED_STEP = 0.1f;
    
    static constexpr float AMOUNT_MIN = 0.0f;
    static constexpr float AMOUNT_MAX = 100.0f;
    static constexpr float AMOUNT_DEFAULT = 50.0f;
    static constexpr float AMOUNT_STEP = 0.1f;
    
    // Enums for discrete parameters
    enum class Mode
    {
        Classic = 0,
        Hard = 1,
        AI = 2
    };
    static constexpr Mode MODE_DEFAULT = Mode::Classic;
    
    enum class Key
    {
        C = 0, CSharp = 1, D = 2, DSharp = 3,
        E = 4, F = 5, FSharp = 6, G = 7,
        GSharp = 8, A = 9, ASharp = 10, B = 11
    };
    static constexpr Key KEY_DEFAULT = Key::C;
    
    enum class Scale
    {
        Major = 0,
        Minor = 1,
        Chromatic = 2
    };
    static constexpr Scale SCALE_DEFAULT = Scale::Major;
    
    // Constructor
    Parameters();
    
    // Create parameter layout for AudioProcessorValueTreeState
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Scale definitions
    static const std::vector<int>& getScaleNotes(Scale scale);
    static String getModeName(Mode mode);
    static String getKeyName(Key key);
    static String getScaleName(Scale scale);
    
private:
    // Scale note definitions (semitone offsets from root)
    static const std::vector<int> majorScale;
    static const std::vector<int> minorScale;
    static const std::vector<int> chromaticScale;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Parameters)
};
