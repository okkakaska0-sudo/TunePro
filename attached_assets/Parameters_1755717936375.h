#pragma once

#include "JuceHeader.h"

class Parameters
{
public:
    // Parameter IDs
    static const String SPEED_ID;
    static const String AMOUNT_ID;
    static const String MODE_ID;
    static const String KEY_ID;
    static const String SCALE_ID;

    // Enums for categorical parameters
    enum class Mode
    {
        Classic = 0,
        Hard = 1,
        AI = 2
    };

    enum class Key
    {
        C = 0, CSharp = 1, D = 2, DSharp = 3,
        E = 4, F = 5, FSharp = 6, G = 7,
        GSharp = 8, A = 9, ASharp = 10, B = 11
    };

    enum class Scale
    {
        Major = 0,
        Minor = 1,
        Chromatic = 2
    };

    Parameters();
    ~Parameters();

    // Create parameter layout for AudioProcessorValueTreeState
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Parameter ranges and defaults
    static constexpr float SPEED_MIN = 0.0f;
    static constexpr float SPEED_MAX = 100.0f;
    static constexpr float SPEED_DEFAULT = 50.0f;
    static constexpr float SPEED_STEP = 0.1f;

    static constexpr float AMOUNT_MIN = 0.0f;
    static constexpr float AMOUNT_MAX = 100.0f;
    static constexpr float AMOUNT_DEFAULT = 50.0f;
    static constexpr float AMOUNT_STEP = 0.1f;

    static constexpr int MODE_DEFAULT = static_cast<int>(Mode::Classic);
    static constexpr int KEY_DEFAULT = static_cast<int>(Key::C);
    static constexpr int SCALE_DEFAULT = static_cast<int>(Scale::Major);

    // Utility functions
    static String getModeString(Mode mode);
    static String getKeyString(Key key);
    static String getScaleString(Scale scale);
    
    static Mode getMode(int value);
    static Key getKey(int value);
    static Scale getScale(int value);

    // Scale note arrays
    static const std::vector<int>& getMajorScale();
    static const std::vector<int>& getMinorScale();
    static const std::vector<int>& getChromaticScale();
    static const std::vector<int>& getScaleNotes(Scale scale);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Parameters)
};
