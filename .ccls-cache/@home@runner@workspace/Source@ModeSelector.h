#pragma once

#include "JuceHeader.h"
#include "Parameters.h"

class ModeSelector : public Component,
                    public ComboBox::Listener
{
public:
    ModeSelector();
    ~ModeSelector() override;

    // Component overrides
    void paint(Graphics& g) override;
    void resized() override;

    // ComboBox::Listener
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

    // Mode management
    void setCurrentMode(Parameters::Mode mode);
    Parameters::Mode getCurrentMode() const { return currentMode; }
    
    // Callbacks
    std::function<void(Parameters::Mode)> onModeChanged;
    
    // Appearance
    void setModeDescriptions(const StringArray& descriptions);
    void setShowModeDescription(bool shouldShow) { showDescription = shouldShow; repaint(); }

private:
    ComboBox modeComboBox;
    Label titleLabel;
    Label descriptionLabel;
    
    Parameters::Mode currentMode = Parameters::Mode::Classic;
    StringArray modeDescriptions;
    bool showDescription = true;
    
    // Mode-specific colors and styling
    struct ModeStyle
    {
        Colour primaryColour;
        Colour accentColour;
        String iconPath;
        
        ModeStyle(const Colour& primary, const Colour& accent, const String& icon)
            : primaryColour(primary), accentColour(accent), iconPath(icon) {}
    };
    
    std::map<Parameters::Mode, ModeStyle> modeStyles;
    
    void setupModeStyles();
    void updateModeDisplay();
    void drawModeIcon(Graphics& g, Rectangle<float> iconBounds, Parameters::Mode mode);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeSelector)
};

class ModeVisualizerComponent : public Component,
                               public Timer
{
public:
    ModeVisualizerComponent();
    ~ModeVisualizerComponent() override;
    
    void paint(Graphics& g) override;
    void timerCallback() override;
    
    void setMode(Parameters::Mode mode);
    void setProcessingLevel(float level) { processingLevel = jlimit(0.0f, 1.0f, level); }
    void setInputSignal(const float* buffer, int numSamples);
    
private:
    Parameters::Mode currentMode = Parameters::Mode::Classic;
    float processingLevel = 0.0f;
    
    // Visualization data
    std::vector<float> waveformData;
    std::vector<float> spectrumData;
    float animationPhase = 0.0f;
    
    void drawClassicVisualization(Graphics& g, Rectangle<float> bounds);
    void drawHardVisualization(Graphics& g, Rectangle<float> bounds);
    void drawAIVisualization(Graphics& g, Rectangle<float> bounds);
    
    void updateWaveformData();
    void updateSpectrumData();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeVisualizerComponent)
};
