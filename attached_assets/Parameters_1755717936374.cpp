#include "Parameters.h"

// Parameter ID strings
const String Parameters::SPEED_ID = "speed";
const String Parameters::AMOUNT_ID = "amount";
const String Parameters::MODE_ID = "mode";
const String Parameters::KEY_ID = "key";
const String Parameters::SCALE_ID = "scale";

Parameters::Parameters()
{
}

Parameters::~Parameters()
{
}

AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    // Speed parameter - Controls how fast the correction is applied
    params.push_back(std::make_unique<AudioParameterFloat>(
        SPEED_ID,
        "Speed",
        NormalisableRange<float>(SPEED_MIN, SPEED_MAX, SPEED_STEP),
        SPEED_DEFAULT,
        String(),
        AudioProcessorParameter::genericParameter,
        [](float value, int) { return String(value, 1) + "%"; }
    ));

    // Amount parameter - Controls the strength of the correction
    params.push_back(std::make_unique<AudioParameterFloat>(
        AMOUNT_ID,
        "Amount",
        NormalisableRange<float>(AMOUNT_MIN, AMOUNT_MAX, AMOUNT_STEP),
        AMOUNT_DEFAULT,
        String(),
        AudioProcessorParameter::genericParameter,
        [](float value, int) { return String(value, 1) + "%"; }
    ));

    // Mode parameter - Selects the correction algorithm
    StringArray modeChoices;
    modeChoices.add("Classic");
    modeChoices.add("Hard");
    modeChoices.add("AI");

    params.push_back(std::make_unique<AudioParameterChoice>(
        MODE_ID,
        "Mode",
        modeChoices,
        MODE_DEFAULT
    ));

    // Key parameter - Sets the musical key
    StringArray keyChoices;
    keyChoices.add("C");
    keyChoices.add("C#");
    keyChoices.add("D");
    keyChoices.add("D#");
    keyChoices.add("E");
    keyChoices.add("F");
    keyChoices.add("F#");
    keyChoices.add("G");
    keyChoices.add("G#");
    keyChoices.add("A");
    keyChoices.add("A#");
    keyChoices.add("B");

    params.push_back(std::make_unique<AudioParameterChoice>(
        KEY_ID,
        "Key",
        keyChoices,
        KEY_DEFAULT
    ));

    // Scale parameter - Sets the musical scale
    StringArray scaleChoices;
    scaleChoices.add("Major");
    scaleChoices.add("Minor");
    scaleChoices.add("Chromatic");

    params.push_back(std::make_unique<AudioParameterChoice>(
        SCALE_ID,
        "Scale",
        scaleChoices,
        SCALE_DEFAULT
    ));

    return { params.begin(), params.end() };
}

String Parameters::getModeString(Mode mode)
{
    switch (mode)
    {
        case Mode::Classic: return "Classic";
        case Mode::Hard: return "Hard";
        case Mode::AI: return "AI";
        default: return "Classic";
    }
}

String Parameters::getKeyString(Key key)
{
    const char* keyNames[] = {
        "C", "C#", "D", "D#", "E", "F", 
        "F#", "G", "G#", "A", "A#", "B"
    };
    
    int keyIndex = static_cast<int>(key);
    if (keyIndex >= 0 && keyIndex < 12)
    {
        return String(keyNames[keyIndex]);
    }
    
    return "C";
}

String Parameters::getScaleString(Scale scale)
{
    switch (scale)
    {
        case Scale::Major: return "Major";
        case Scale::Minor: return "Minor";
        case Scale::Chromatic: return "Chromatic";
        default: return "Major";
    }
}

Parameters::Mode Parameters::getMode(int value)
{
    if (value >= 0 && value <= 2)
        return static_cast<Mode>(value);
    return Mode::Classic;
}

Parameters::Key Parameters::getKey(int value)
{
    if (value >= 0 && value <= 11)
        return static_cast<Key>(value);
    return Key::C;
}

Parameters::Scale Parameters::getScale(int value)
{
    if (value >= 0 && value <= 2)
        return static_cast<Scale>(value);
    return Scale::Major;
}

const std::vector<int>& Parameters::getMajorScale()
{
    static const std::vector<int> majorScale = {0, 2, 4, 5, 7, 9, 11}; // Major scale intervals
    return majorScale;
}

const std::vector<int>& Parameters::getMinorScale()
{
    static const std::vector<int> minorScale = {0, 2, 3, 5, 7, 8, 10}; // Natural minor scale intervals
    return minorScale;
}

const std::vector<int>& Parameters::getChromaticScale()
{
    static const std::vector<int> chromaticScale = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}; // All semitones
    return chromaticScale;
}

const std::vector<int>& Parameters::getScaleNotes(Scale scale)
{
    switch (scale)
    {
        case Scale::Major: return getMajorScale();
        case Scale::Minor: return getMinorScale();
        case Scale::Chromatic: return getChromaticScale();
        default: return getMajorScale();
    }
}
