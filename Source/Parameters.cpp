#include "Parameters.h"

// Parameter IDs
const String Parameters::SPEED_ID = "speed";
const String Parameters::AMOUNT_ID = "amount";
const String Parameters::MODE_ID = "mode";
const String Parameters::KEY_ID = "key";
const String Parameters::SCALE_ID = "scale";

// Scale definitions (semitone offsets from root)
const std::vector<int> Parameters::majorScale = {0, 2, 4, 5, 7, 9, 11};
const std::vector<int> Parameters::minorScale = {0, 2, 3, 5, 7, 8, 10};
const std::vector<int> Parameters::chromaticScale = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

Parameters::Parameters()
{
}

AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> parameters;
    
    // Speed parameter
    parameters.push_back(std::make_unique<AudioParameterFloat>(
        SPEED_ID,
        "Speed",
        NormalisableRange<float>(SPEED_MIN, SPEED_MAX, SPEED_STEP),
        SPEED_DEFAULT,
        "Speed",
        AudioProcessorParameter::genericParameter,
        [](float value, int) { return String(value, 1) + " %"; }
    ));
    
    // Amount parameter
    parameters.push_back(std::make_unique<AudioParameterFloat>(
        AMOUNT_ID,
        "Amount",
        NormalisableRange<float>(AMOUNT_MIN, AMOUNT_MAX, AMOUNT_STEP),
        AMOUNT_DEFAULT,
        "Amount",
        AudioProcessorParameter::genericParameter,
        [](float value, int) { return String(value, 1) + " %"; }
    ));
    
    // Mode parameter
    parameters.push_back(std::make_unique<AudioParameterChoice>(
        MODE_ID,
        "Mode",
        StringArray{"Classic", "Hard", "AI"},
        static_cast<int>(MODE_DEFAULT)
    ));
    
    // Key parameter
    parameters.push_back(std::make_unique<AudioParameterChoice>(
        KEY_ID,
        "Key",
        StringArray{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"},
        static_cast<int>(KEY_DEFAULT)
    ));
    
    // Scale parameter
    parameters.push_back(std::make_unique<AudioParameterChoice>(
        SCALE_ID,
        "Scale",
        StringArray{"Major", "Minor", "Chromatic"},
        static_cast<int>(SCALE_DEFAULT)
    ));
    
    return {parameters.begin(), parameters.end()};
}

const std::vector<int>& Parameters::getScaleNotes(Scale scale)
{
    switch (scale)
    {
        case Scale::Major:
            return majorScale;
        case Scale::Minor:
            return minorScale;
        case Scale::Chromatic:
            return chromaticScale;
        default:
            return majorScale;
    }
}

String Parameters::getModeName(Mode mode)
{
    switch (mode)
    {
        case Mode::Classic: return "Classic";
        case Mode::Hard: return "Hard";
        case Mode::AI: return "AI";
        default: return "Classic";
    }
}

String Parameters::getKeyName(Key key)
{
    const char* keyNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    return keyNames[static_cast<int>(key)];
}

String Parameters::getScaleName(Scale scale)
{
    switch (scale)
    {
        case Scale::Major: return "Major";
        case Scale::Minor: return "Minor";
        case Scale::Chromatic: return "Chromatic";
        default: return "Major";
    }
}
