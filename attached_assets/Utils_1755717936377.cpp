#include "Utils.h"
#include <algorithm>
#include <numeric>

// Initialize static lookup tables
std::vector<float> Utils::sinLookupTable(LOOKUP_TABLE_SIZE);
std::vector<float> Utils::cosLookupTable(LOOKUP_TABLE_SIZE);

float Utils::frequencyToMidiNote(float frequency)
{
    if (frequency <= 0.0f)
        return 0.0f;
    
    return MIDI_A4 + 12.0f * std::log2(frequency / CONCERT_A_FREQ);
}

float Utils::midiNoteToFrequency(float midiNote)
{
    return CONCERT_A_FREQ * std::pow(2.0f, (midiNote - MIDI_A4) / 12.0f);
}

float Utils::quantizeToScale(float midiNote, Parameters::Key key, Parameters::Scale scale)
{
    if (midiNote <= 0.0f)
        return midiNote;
    
    // Get the scale notes for the specified scale
    const auto& scaleNotes = Parameters::getScaleNotes(scale);
    int keyOffset = static_cast<int>(key);
    
    // Find the nearest scale note
    int quantizedNote = findNearestScaleNote(midiNote, scaleNotes, keyOffset);
    
    return static_cast<float>(quantizedNote);
}

int Utils::findNearestScaleNote(float midiNote, const std::vector<int>& scaleNotes, int keyOffset)
{
    int noteInt = static_cast<int>(std::round(midiNote));
    int octave = noteInt / 12;
    int noteInOctave = noteInt % 12;
    
    // Handle negative notes
    if (noteInOctave < 0)
    {
        noteInOctave += 12;
        octave--;
    }
    
    // Find the closest note in the scale
    int closestNote = 0;
    float minDistance = 12.0f;
    
    for (int scaleNote : scaleNotes)
    {
        int scaledNote = (scaleNote + keyOffset) % 12;
        
        // Check distance in both directions (considering wrap-around)
        float distance1 = std::abs(noteInOctave - scaledNote);
        float distance2 = 12.0f - distance1;
        float distance = std::min(distance1, distance2);
        
        if (distance < minDistance)
        {
            minDistance = distance;
            closestNote = scaledNote;
        }
    }
    
    return octave * 12 + closestNote;
}

float Utils::linearToDecibels(float linearValue)
{
    if (linearValue <= 0.0f)
        return -100.0f; // Effective negative infinity
    
    return 20.0f * std::log10(linearValue);
}

float Utils::decibelsToLinear(float decibels)
{
    return std::pow(10.0f, decibels / 20.0f);
}

float Utils::rmsToDecibels(float rmsValue)
{
    return linearToDecibels(rmsValue);
}

float Utils::linearInterpolation(float x1, float y1, float x2, float y2, float x)
{
    if (std::abs(x2 - x1) < 1e-6f)
        return y1;
    
    float t = (x - x1) / (x2 - x1);
    return y1 + t * (y2 - y1);
}

float Utils::cubicInterpolation(float y0, float y1, float y2, float y3, float x)
{
    float a = y3 - y2 - y0 + y1;
    float b = y0 - y1 - a;
    float c = y2 - y0;
    float d = y1;
    
    return a * x * x * x + b * x * x + c * x + d;
}

float Utils::hermiteInterpolation(float y0, float y1, float y2, float y3, float x)
{
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    
    return ((c3 * x + c2) * x + c1) * x + c0;
}

void Utils::applyWindow(float* buffer, int numSamples, WindowType windowType)
{
    switch (windowType)
    {
        case WindowType::Rectangular:
            // No modification needed
            break;
            
        case WindowType::Hann:
            for (int i = 0; i < numSamples; ++i)
            {
                float window = 0.5f * (1.0f - std::cos(TWO_PI * i / (numSamples - 1)));
                buffer[i] *= window;
            }
            break;
            
        case WindowType::Hamming:
            for (int i = 0; i < numSamples; ++i)
            {
                float window = 0.54f - 0.46f * std::cos(TWO_PI * i / (numSamples - 1));
                buffer[i] *= window;
            }
            break;
            
        case WindowType::Blackman:
            for (int i = 0; i < numSamples; ++i)
            {
                float phase = TWO_PI * i / (numSamples - 1);
                float window = 0.42f - 0.5f * std::cos(phase) + 0.08f * std::cos(2.0f * phase);
                buffer[i] *= window;
            }
            break;
            
        case WindowType::Kaiser:
            // Simplified Kaiser window (beta = 5.0)
            float beta = 5.0f;
            float alpha = (numSamples - 1) * 0.5f;
            
            for (int i = 0; i < numSamples; ++i)
            {
                float x = (i - alpha) / alpha;
                float window = std::cosh(beta * std::sqrt(1.0f - x * x)) / std::cosh(beta);
                buffer[i] *= window;
            }
            break;
    }
}

float Utils::detectPitchZeroCrossing(const float* buffer, int numSamples, float sampleRate)
{
    if (numSamples < 2)
        return 0.0f;
    
    std::vector<int> zeroCrossings;
    
    // Find zero crossings
    for (int i = 1; i < numSamples; ++i)
    {
        if ((buffer[i-1] < 0.0f && buffer[i] >= 0.0f) ||
            (buffer[i-1] >= 0.0f && buffer[i] < 0.0f))
        {
            zeroCrossings.push_back(i);
        }
    }
    
    if (zeroCrossings.size() < 2)
        return 0.0f;
    
    // Calculate average period between zero crossings
    float totalPeriod = 0.0f;
    int periodCount = 0;
    
    for (size_t i = 2; i < zeroCrossings.size(); i += 2) // Use every other crossing for full periods
    {
        float period = static_cast<float>(zeroCrossings[i] - zeroCrossings[i-2]);
        totalPeriod += period;
        periodCount++;
    }
    
    if (periodCount == 0)
        return 0.0f;
    
    float averagePeriod = totalPeriod / periodCount;
    return sampleRate / averagePeriod;
}

float Utils::calculateSpectralCentroid(const std::vector<float>& magnitude, float sampleRate)
{
    if (magnitude.empty())
        return 0.0f;
    
    float numerator = 0.0f;
    float denominator = 0.0f;
    
    for (size_t i = 1; i < magnitude.size(); ++i)
    {
        float frequency = static_cast<float>(i) * sampleRate / (2.0f * (magnitude.size() - 1));
        numerator += frequency * magnitude[i];
        denominator += magnitude[i];
    }
    
    return denominator > 0.0f ? numerator / denominator : 0.0f;
}

std::vector<int> Utils::findSpectralPeaks(const std::vector<float>& magnitude, float threshold)
{
    std::vector<int> peaks;
    
    if (magnitude.size() < 3)
        return peaks;
    
    for (size_t i = 1; i < magnitude.size() - 1; ++i)
    {
        if (magnitude[i] > magnitude[i-1] && 
            magnitude[i] > magnitude[i+1] && 
            magnitude[i] > threshold)
        {
            peaks.push_back(static_cast<int>(i));
        }
    }
    
    return peaks;
}

String Utils::noteNumberToNoteName(int noteNumber)
{
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    
    int octave = noteNumber / 12 - 1; // MIDI note 60 is C4
    int note = noteNumber % 12;
    
    if (note < 0)
    {
        note += 12;
        octave--;
    }
    
    return String(noteNames[note]) + String(octave);
}

int Utils::noteNameToNoteNumber(const String& noteName)
{
    // Simple implementation - could be expanded for more robust parsing
    if (noteName.length() < 2)
        return 60; // Default to C4
    
    char noteLetter = noteName[0];
    int noteOffset = 0;
    
    switch (noteLetter)
    {
        case 'C': noteOffset = 0; break;
        case 'D': noteOffset = 2; break;
        case 'E': noteOffset = 4; break;
        case 'F': noteOffset = 5; break;
        case 'G': noteOffset = 7; break;
        case 'A': noteOffset = 9; break;
        case 'B': noteOffset = 11; break;
        default: return 60;
    }
    
    // Check for sharp
    int index = 1;
    if (noteName.length() > 2 && noteName[1] == '#')
    {
        noteOffset++;
        index = 2;
    }
    
    // Get octave
    int octave = noteName.substring(index).getIntValue();
    
    return (octave + 1) * 12 + noteOffset;
}

float Utils::centsToRatio(float cents)
{
    return std::pow(2.0f, cents / 1200.0f);
}

float Utils::ratioToCents(float ratio)
{
    if (ratio <= 0.0f)
        return 0.0f;
    
    return 1200.0f * std::log2(ratio);
}

void Utils::normalize(float* buffer, int numSamples, float targetLevel)
{
    if (numSamples <= 0)
        return;
    
    // Find peak
    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        peak = std::max(peak, std::abs(buffer[i]));
    }
    
    if (peak > 0.0f)
    {
        float scale = targetLevel / peak;
        for (int i = 0; i < numSamples; ++i)
        {
            buffer[i] *= scale;
        }
    }
}

float Utils::calculateCorrelation(const float* buffer1, const float* buffer2, int numSamples)
{
    if (numSamples <= 0)
        return 0.0f;
    
    float sum1 = 0.0f, sum2 = 0.0f;
    float sum1Sq = 0.0f, sum2Sq = 0.0f;
    float sumProduct = 0.0f;
    
    for (int i = 0; i < numSamples; ++i)
    {
        sum1 += buffer1[i];
        sum2 += buffer2[i];
        sum1Sq += buffer1[i] * buffer1[i];
        sum2Sq += buffer2[i] * buffer2[i];
        sumProduct += buffer1[i] * buffer2[i];
    }
    
    float n = static_cast<float>(numSamples);
    float numerator = n * sumProduct - sum1 * sum2;
    float denominator = std::sqrt((n * sum1Sq - sum1 * sum1) * (n * sum2Sq - sum2 * sum2));
    
    return denominator > 0.0f ? numerator / denominator : 0.0f;
}

void Utils::fadeInOut(float* buffer, int numSamples, int fadeLength)
{
    if (fadeLength <= 0 || numSamples <= 0)
        return;
    
    fadeLength = std::min(fadeLength, numSamples / 2);
    
    // Fade in
    for (int i = 0; i < fadeLength; ++i)
    {
        float fade = static_cast<float>(i) / fadeLength;
        buffer[i] *= fade;
    }
    
    // Fade out
    for (int i = numSamples - fadeLength; i < numSamples; ++i)
    {
        float fade = static_cast<float>(numSamples - 1 - i) / fadeLength;
        buffer[i] *= fade;
    }
}

// Fast trigonometric functions using lookup tables
void Utils::initializeLookupTables()
{
    static bool initialized = false;
    if (initialized)
        return;
    
    for (int i = 0; i < LOOKUP_TABLE_SIZE; ++i)
    {
        float angle = TWO_PI * i / LOOKUP_TABLE_SIZE;
        sinLookupTable[i] = std::sin(angle);
        cosLookupTable[i] = std::cos(angle);
    }
    
    initialized = true;
}

float Utils::lookupSin(float x)
{
    initializeLookupTables();
    
    // Normalize to [0, 2π)
    x = std::fmod(x, TWO_PI);
    if (x < 0.0f)
        x += TWO_PI;
    
    // Convert to table index
    float indexFloat = x * LOOKUP_TABLE_SIZE * INV_TWO_PI;
    size_t index = static_cast<size_t>(indexFloat);
    float fraction = indexFloat - static_cast<float>(index);
    
    // Linear interpolation between adjacent values
    size_t nextIndex = (index + 1) % LOOKUP_TABLE_SIZE;
    return sinLookupTable[index] + fraction * (sinLookupTable[nextIndex] - sinLookupTable[index]);
}

float Utils::lookupCos(float x)
{
    initializeLookupTables();
    
    // Normalize to [0, 2π)
    x = std::fmod(x, TWO_PI);
    if (x < 0.0f)
        x += TWO_PI;
    
    // Convert to table index
    float indexFloat = x * LOOKUP_TABLE_SIZE * INV_TWO_PI;
    size_t index = static_cast<size_t>(indexFloat);
    float fraction = indexFloat - static_cast<float>(index);
    
    // Linear interpolation between adjacent values
    size_t nextIndex = (index + 1) % LOOKUP_TABLE_SIZE;
    return cosLookupTable[index] + fraction * (cosLookupTable[nextIndex] - cosLookupTable[index]);
}

float Utils::fastSin(float x)
{
    return lookupSin(x);
}

float Utils::fastCos(float x)
{
    return lookupCos(x);
}

float Utils::fastAtan2(float y, float x)
{
    // Fast approximation of atan2
    if (x == 0.0f)
    {
        return (y > 0.0f) ? HALF_PI : -HALF_PI;
    }
    
    float atan = y / x;
    float absAtan = std::abs(atan);
    
    // Polynomial approximation
    float result = HALF_PI - atan / (1.0f + 0.28f * absAtan);
    
    if (x < 0.0f)
    {
        result = (y >= 0.0f) ? result + PI : result - PI;
    }
    
    return result;
}

float Utils::fastLog2(float x)
{
    // Fast log2 approximation using bit manipulation
    if (x <= 0.0f)
        return -100.0f; // Approximation of -infinity
    
    union { float f; int i; } u;
    u.f = x;
    
    float exponent = static_cast<float>((u.i >> 23) & 0xFF) - 127.0f;
    u.i = (u.i & 0x007FFFFF) | 0x3F800000; // Keep mantissa, set exponent to 0
    
    // Polynomial approximation for mantissa
    float mantissa = u.f;
    float mantissaLog = -1.49278f + (2.11263f + (-0.729104f + 0.10969f * mantissa) * mantissa) * mantissa;
    
    return exponent + mantissaLog;
}

float Utils::fastPow(float base, float exponent)
{
    if (base <= 0.0f)
        return 0.0f;
    
    return std::exp(exponent * std::log(base)); // Could be optimized further with lookup tables
}
