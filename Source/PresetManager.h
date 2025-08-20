#pragma once

#include "JuceHeader.h"
#include <vector>

class PresetManager
{
public:
    struct Preset
    {
        String name;
        float speed;
        float amount;
        int mode;
        int key;
        int scale;
        String description;
        Time dateCreated;
        
        Preset() : speed(50.0f), amount(50.0f), mode(0), key(0), scale(0) {}
        
        Preset(const String& presetName, float speedVal, float amountVal, 
               int modeVal, int keyVal, int scaleVal, const String& desc = String())
            : name(presetName), speed(speedVal), amount(amountVal), mode(modeVal), 
              key(keyVal), scale(scaleVal), description(desc), dateCreated(Time::getCurrentTime()) {}
    };

    PresetManager(AudioProcessorValueTreeState& parameters);
    ~PresetManager();

    // Preset management
    void savePreset(const String& name, const String& description = String());
    bool loadPreset(int index);
    bool loadPreset(const String& name);
    void deletePreset(int index);
    void deletePreset(const String& name);
    
    // Factory presets
    void loadFactoryPresets();
    void resetToDefaults();
    
    // Preset access
    int getNumPresets() const { return static_cast<int>(presets.size()); }
    const Preset& getPreset(int index) const;
    StringArray getPresetNames() const;
    int getCurrentPresetIndex() const { return currentPresetIndex; }
    
    // File operations
    void savePresetsToFile();
    void loadPresetsFromFile();
    File getPresetDirectory() const;
    File getPresetFile() const;
    
    // Import/Export
    bool exportPreset(int index, const File& file);
    bool importPreset(const File& file);
    bool exportAllPresets(const File& file);
    bool importPresetsFromFile(const File& file);
    
    // Validation
    bool isValidPresetIndex(int index) const;
    bool presetExists(const String& name) const;
    
    // Callbacks
    std::function<void()> onPresetChanged;
    std::function<void(const String&)> onPresetSaved;
    std::function<void(const String&)> onPresetDeleted;

private:
    AudioProcessorValueTreeState& parameters;
    std::vector<Preset> presets;
    int currentPresetIndex = -1;
    
    // File management
    static const String presetFileExtension;
    static const String presetFileName;
    
    // Helper methods
    Preset createPresetFromCurrentState(const String& name, const String& description = String());
    void applyPresetToParameters(const Preset& preset);
    ValueTree presetToValueTree(const Preset& preset);
    Preset valueTreeToPreset(const ValueTree& tree);
    
    // Factory preset definitions
    void addFactoryPreset(const String& name, float speed, float amount, int mode, int key, int scale, const String& description);
    
    // File I/O helpers
    bool writePresetToXml(const Preset& preset, XmlElement& xml);
    bool readPresetFromXml(const XmlElement& xml, Preset& preset);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
