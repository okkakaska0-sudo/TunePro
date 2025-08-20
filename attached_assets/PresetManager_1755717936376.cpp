#include "PresetManager.h"
#include "Parameters.h"

const String PresetManager::presetFileExtension = ".xml";
const String PresetManager::presetFileName = "ProAutoTunePresets.xml";

PresetManager::PresetManager(AudioProcessorValueTreeState& params)
    : parameters(params), currentPresetIndex(-1)
{
    loadFactoryPresets();
    loadPresetsFromFile();
}

PresetManager::~PresetManager()
{
    savePresetsToFile();
}

void PresetManager::savePreset(const String& name, const String& description)
{
    if (name.isEmpty())
        return;
    
    // Check if preset with this name already exists
    int existingIndex = -1;
    for (int i = 0; i < static_cast<int>(presets.size()); ++i)
    {
        if (presets[i].name == name)
        {
            existingIndex = i;
            break;
        }
    }
    
    Preset newPreset = createPresetFromCurrentState(name, description);
    
    if (existingIndex >= 0)
    {
        // Replace existing preset
        presets[existingIndex] = newPreset;
        currentPresetIndex = existingIndex;
    }
    else
    {
        // Add new preset
        presets.push_back(newPreset);
        currentPresetIndex = static_cast<int>(presets.size()) - 1;
    }
    
    savePresetsToFile();
    
    if (onPresetSaved)
        onPresetSaved(name);
}

bool PresetManager::loadPreset(int index)
{
    if (!isValidPresetIndex(index))
        return false;
    
    const auto& preset = presets[index];
    applyPresetToParameters(preset);
    currentPresetIndex = index;
    
    if (onPresetChanged)
        onPresetChanged();
    
    return true;
}

bool PresetManager::loadPreset(const String& name)
{
    for (int i = 0; i < static_cast<int>(presets.size()); ++i)
    {
        if (presets[i].name == name)
        {
            return loadPreset(i);
        }
    }
    return false;
}

void PresetManager::deletePreset(int index)
{
    if (!isValidPresetIndex(index))
        return;
    
    String deletedName = presets[index].name;
    presets.erase(presets.begin() + index);
    
    if (currentPresetIndex == index)
    {
        currentPresetIndex = -1;
    }
    else if (currentPresetIndex > index)
    {
        currentPresetIndex--;
    }
    
    savePresetsToFile();
    
    if (onPresetDeleted)
        onPresetDeleted(deletedName);
}

void PresetManager::deletePreset(const String& name)
{
    for (int i = 0; i < static_cast<int>(presets.size()); ++i)
    {
        if (presets[i].name == name)
        {
            deletePreset(i);
            break;
        }
    }
}

void PresetManager::loadFactoryPresets()
{
    presets.clear();
    
    // Add factory presets with professional settings
    addFactoryPreset("Default", 50.0f, 50.0f, 0, 0, 0, "Balanced correction for most vocals");
    addFactoryPreset("Vocal Classic", 30.0f, 70.0f, 0, 0, 0, "Smooth, natural vocal correction");
    addFactoryPreset("Hard Tune", 90.0f, 85.0f, 1, 0, 0, "Aggressive T-Pain style effect");
    addFactoryPreset("AI Natural", 25.0f, 60.0f, 2, 0, 0, "AI-powered natural correction");
    addFactoryPreset("Pop Vocal", 40.0f, 75.0f, 0, 0, 0, "Perfect for pop vocals");
    addFactoryPreset("Rap Vocal", 80.0f, 90.0f, 1, 0, 2, "Hard correction for rap vocals");
    addFactoryPreset("Choir", 20.0f, 40.0f, 2, 0, 0, "Gentle correction for choir vocals");
    addFactoryPreset("Robot Voice", 100.0f, 100.0f, 1, 0, 2, "Full robotic effect");
    addFactoryPreset("Subtle Fix", 15.0f, 30.0f, 0, 0, 0, "Very gentle pitch correction");
    addFactoryPreset("Major Scale Fix", 60.0f, 80.0f, 0, 0, 0, "Strong correction to major scale");
    addFactoryPreset("Minor Blues", 45.0f, 65.0f, 0, 9, 1, "A minor scale correction");
    addFactoryPreset("Chromatic", 70.0f, 50.0f, 0, 0, 2, "Chromatic scale correction");
}

void PresetManager::resetToDefaults()
{
    parameters.getParameter(Parameters::SPEED_ID)->setValueNotifyingHost(
        parameters.getParameterRange(Parameters::SPEED_ID).convertTo0to1(Parameters::SPEED_DEFAULT));
    parameters.getParameter(Parameters::AMOUNT_ID)->setValueNotifyingHost(
        parameters.getParameterRange(Parameters::AMOUNT_ID).convertTo0to1(Parameters::AMOUNT_DEFAULT));
    parameters.getParameter(Parameters::MODE_ID)->setValueNotifyingHost(
        static_cast<float>(Parameters::MODE_DEFAULT) / 2.0f);
    parameters.getParameter(Parameters::KEY_ID)->setValueNotifyingHost(
        static_cast<float>(Parameters::KEY_DEFAULT) / 11.0f);
    parameters.getParameter(Parameters::SCALE_ID)->setValueNotifyingHost(
        static_cast<float>(Parameters::SCALE_DEFAULT) / 2.0f);
    
    currentPresetIndex = -1;
    
    if (onPresetChanged)
        onPresetChanged();
}

const PresetManager::Preset& PresetManager::getPreset(int index) const
{
    static Preset emptyPreset;
    
    if (isValidPresetIndex(index))
        return presets[index];
    
    return emptyPreset;
}

StringArray PresetManager::getPresetNames() const
{
    StringArray names;
    for (const auto& preset : presets)
    {
        names.add(preset.name);
    }
    return names;
}

void PresetManager::savePresetsToFile()
{
    auto presetFile = getPresetFile();
    
    XmlElement root("ProAutoTunePresets");
    root.setAttribute("version", "1.0");
    root.setAttribute("count", static_cast<int>(presets.size()));
    
    for (const auto& preset : presets)
    {
        auto presetElement = root.createNewChildElement("Preset");
        writePresetToXml(preset, *presetElement);
    }
    
    if (!root.writeTo(presetFile))
    {
        DBG("Failed to save presets to file: " + presetFile.getFullPathName());
    }
}

void PresetManager::loadPresetsFromFile()
{
    auto presetFile = getPresetFile();
    
    if (!presetFile.existsAsFile())
        return;
    
    auto xml = XmlDocument::parse(presetFile);
    if (xml == nullptr)
        return;
    
    if (!xml->hasTagName("ProAutoTunePresets"))
        return;
    
    // Clear existing user presets but keep factory presets
    std::vector<Preset> factoryPresets;
    for (const auto& preset : presets)
    {
        // Assume factory presets are the first ones loaded
        if (preset.description.contains("factory") || 
            preset.name == "Default" || preset.name == "Vocal Classic" ||
            preset.name == "Hard Tune" || preset.name == "AI Natural")
        {
            factoryPresets.push_back(preset);
        }
    }
    
    presets = factoryPresets;
    
    // Load user presets
    for (auto* presetElement : xml->getChildIterator())
    {
        if (presetElement->hasTagName("Preset"))
        {
            Preset preset;
            if (readPresetFromXml(*presetElement, preset))
            {
                // Only add if not a factory preset
                bool isFactory = false;
                for (const auto& factoryPreset : factoryPresets)
                {
                    if (factoryPreset.name == preset.name)
                    {
                        isFactory = true;
                        break;
                    }
                }
                
                if (!isFactory)
                {
                    presets.push_back(preset);
                }
            }
        }
    }
}

File PresetManager::getPresetDirectory() const
{
    auto userDocsDir = File::getSpecialLocation(File::userDocumentsDirectory);
    auto presetDir = userDocsDir.getChildFile("ProAutoTune").getChildFile("Presets");
    
    if (!presetDir.exists())
        presetDir.createDirectory();
    
    return presetDir;
}

File PresetManager::getPresetFile() const
{
    return getPresetDirectory().getChildFile(presetFileName);
}

bool PresetManager::exportPreset(int index, const File& file)
{
    if (!isValidPresetIndex(index))
        return false;
    
    const auto& preset = presets[index];
    
    XmlElement root("ProAutoTunePreset");
    root.setAttribute("version", "1.0");
    
    auto presetElement = root.createNewChildElement("Preset");
    writePresetToXml(preset, *presetElement);
    
    return root.writeTo(file);
}

bool PresetManager::importPreset(const File& file)
{
    if (!file.existsAsFile())
        return false;
    
    auto xml = XmlDocument::parse(file);
    if (xml == nullptr || !xml->hasTagName("ProAutoTunePreset"))
        return false;
    
    auto presetElement = xml->getChildByName("Preset");
    if (presetElement == nullptr)
        return false;
    
    Preset preset;
    if (readPresetFromXml(*presetElement, preset))
    {
        // Check for name conflicts
        String originalName = preset.name;
        int counter = 1;
        while (presetExists(preset.name))
        {
            preset.name = originalName + " (" + String(counter++) + ")";
        }
        
        presets.push_back(preset);
        savePresetsToFile();
        return true;
    }
    
    return false;
}

bool PresetManager::exportAllPresets(const File& file)
{
    XmlElement root("ProAutoTunePresets");
    root.setAttribute("version", "1.0");
    root.setAttribute("count", static_cast<int>(presets.size()));
    
    for (const auto& preset : presets)
    {
        auto presetElement = root.createNewChildElement("Preset");
        writePresetToXml(preset, *presetElement);
    }
    
    return root.writeTo(file);
}

bool PresetManager::importPresetsFromFile(const File& file)
{
    if (!file.existsAsFile())
        return false;
    
    auto xml = XmlDocument::parse(file);
    if (xml == nullptr || !xml->hasTagName("ProAutoTunePresets"))
        return false;
    
    int importedCount = 0;
    
    for (auto* presetElement : xml->getChildIterator())
    {
        if (presetElement->hasTagName("Preset"))
        {
            Preset preset;
            if (readPresetFromXml(*presetElement, preset))
            {
                // Handle name conflicts
                String originalName = preset.name;
                int counter = 1;
                while (presetExists(preset.name))
                {
                    preset.name = originalName + " (" + String(counter++) + ")";
                }
                
                presets.push_back(preset);
                importedCount++;
            }
        }
    }
    
    if (importedCount > 0)
    {
        savePresetsToFile();
        return true;
    }
    
    return false;
}

bool PresetManager::isValidPresetIndex(int index) const
{
    return index >= 0 && index < static_cast<int>(presets.size());
}

bool PresetManager::presetExists(const String& name) const
{
    for (const auto& preset : presets)
    {
        if (preset.name == name)
            return true;
    }
    return false;
}

PresetManager::Preset PresetManager::createPresetFromCurrentState(const String& name, const String& description)
{
    Preset preset;
    preset.name = name;
    preset.description = description;
    preset.dateCreated = Time::getCurrentTime();
    
    preset.speed = *parameters.getRawParameterValue(Parameters::SPEED_ID);
    preset.amount = *parameters.getRawParameterValue(Parameters::AMOUNT_ID);
    preset.mode = static_cast<int>(*parameters.getRawParameterValue(Parameters::MODE_ID));
    preset.key = static_cast<int>(*parameters.getRawParameterValue(Parameters::KEY_ID));
    preset.scale = static_cast<int>(*parameters.getRawParameterValue(Parameters::SCALE_ID));
    
    return preset;
}

void PresetManager::applyPresetToParameters(const Preset& preset)
{
    auto speedRange = parameters.getParameterRange(Parameters::SPEED_ID);
    auto amountRange = parameters.getParameterRange(Parameters::AMOUNT_ID);
    
    parameters.getParameter(Parameters::SPEED_ID)->setValueNotifyingHost(
        speedRange.convertTo0to1(preset.speed));
    parameters.getParameter(Parameters::AMOUNT_ID)->setValueNotifyingHost(
        amountRange.convertTo0to1(preset.amount));
    parameters.getParameter(Parameters::MODE_ID)->setValueNotifyingHost(
        static_cast<float>(preset.mode) / 2.0f);
    parameters.getParameter(Parameters::KEY_ID)->setValueNotifyingHost(
        static_cast<float>(preset.key) / 11.0f);
    parameters.getParameter(Parameters::SCALE_ID)->setValueNotifyingHost(
        static_cast<float>(preset.scale) / 2.0f);
}

ValueTree PresetManager::presetToValueTree(const Preset& preset)
{
    ValueTree tree("Preset");
    tree.setProperty("name", preset.name, nullptr);
    tree.setProperty("description", preset.description, nullptr);
    tree.setProperty("speed", preset.speed, nullptr);
    tree.setProperty("amount", preset.amount, nullptr);
    tree.setProperty("mode", preset.mode, nullptr);
    tree.setProperty("key", preset.key, nullptr);
    tree.setProperty("scale", preset.scale, nullptr);
    tree.setProperty("dateCreated", preset.dateCreated.toISO8601(true), nullptr);
    
    return tree;
}

PresetManager::Preset PresetManager::valueTreeToPreset(const ValueTree& tree)
{
    Preset preset;
    preset.name = tree.getProperty("name", "Untitled");
    preset.description = tree.getProperty("description", "");
    preset.speed = tree.getProperty("speed", 50.0f);
    preset.amount = tree.getProperty("amount", 50.0f);
    preset.mode = tree.getProperty("mode", 0);
    preset.key = tree.getProperty("key", 0);
    preset.scale = tree.getProperty("scale", 0);
    
    String dateString = tree.getProperty("dateCreated", "");
    if (dateString.isNotEmpty())
    {
        preset.dateCreated = Time::fromISO8601(dateString);
    }
    else
    {
        preset.dateCreated = Time::getCurrentTime();
    }
    
    return preset;
}

void PresetManager::addFactoryPreset(const String& name, float speed, float amount, 
                                    int mode, int key, int scale, const String& description)
{
    Preset preset(name, speed, amount, mode, key, scale, description);
    presets.push_back(preset);
}

bool PresetManager::writePresetToXml(const Preset& preset, XmlElement& xml)
{
    xml.setAttribute("name", preset.name);
    xml.setAttribute("description", preset.description);
    xml.setAttribute("speed", preset.speed);
    xml.setAttribute("amount", preset.amount);
    xml.setAttribute("mode", preset.mode);
    xml.setAttribute("key", preset.key);
    xml.setAttribute("scale", preset.scale);
    xml.setAttribute("dateCreated", preset.dateCreated.toISO8601(true));
    
    return true;
}

bool PresetManager::readPresetFromXml(const XmlElement& xml, Preset& preset)
{
    if (!xml.hasAttribute("name"))
        return false;
    
    preset.name = xml.getStringAttribute("name");
    preset.description = xml.getStringAttribute("description");
    preset.speed = static_cast<float>(xml.getDoubleAttribute("speed", 50.0));
    preset.amount = static_cast<float>(xml.getDoubleAttribute("amount", 50.0));
    preset.mode = xml.getIntAttribute("mode", 0);
    preset.key = xml.getIntAttribute("key", 0);
    preset.scale = xml.getIntAttribute("scale", 0);
    
    String dateString = xml.getStringAttribute("dateCreated");
    if (dateString.isNotEmpty())
    {
        preset.dateCreated = Time::fromISO8601(dateString);
    }
    else
    {
        preset.dateCreated = Time::getCurrentTime();
    }
    
    return true;
}
