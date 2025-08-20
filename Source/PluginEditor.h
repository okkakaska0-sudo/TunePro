#pragma once

#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "LookAndFeel.h"

class AutoTuneAudioProcessorEditor : public AudioProcessorEditor,
                                     public Timer,
                                     public Button::Listener
{
public:
    AutoTuneAudioProcessorEditor(AutoTuneAudioProcessor&);
    ~AutoTuneAudioProcessorEditor() override;

    void paint(Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void buttonClicked(Button* button) override;

private:
    AutoTuneAudioProcessor& audioProcessor;
    
    // Custom Look and Feel
    ProAutoTuneLookAndFeel lookAndFeel;
    
    // Main controls
    Slider speedSlider;
    Label speedLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> speedAttachment;
    
    Slider amountSlider;
    Label amountLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> amountAttachment;
    
    ComboBox modeSelector;
    Label modeLabel;
    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    
    ComboBox keySelector;
    Label keyLabel;
    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> keyAttachment;
    
    ComboBox scaleSelector;
    Label scaleLabel;
    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> scaleAttachment;
    
    // Preset controls
    TextButton savePresetButton;
    TextButton loadPresetButton;
    ComboBox presetSelector;
    
    // Visual elements
    Rectangle<int> headerArea;
    Rectangle<int> controlsArea;
    Rectangle<int> presetArea;
    
    // Real-time display
    float currentInputLevel = 0.0f;
    float currentOutputLevel = 0.0f;
    std::vector<float> pitchHistory;
    static constexpr int pitchHistorySize = 100;
    
    // Helper methods
    void setupControls();
    void setupLayout();
    void updatePresetList();
    void drawHeader(Graphics& g, const Rectangle<int>& area);
    void drawControls(Graphics& g, const Rectangle<int>& area);
    void drawPresetSection(Graphics& g, const Rectangle<int>& area);
    void drawPitchDisplay(Graphics& g, const Rectangle<int>& area);
    void drawLevelMeters(Graphics& g, const Rectangle<int>& area);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutoTuneAudioProcessorEditor)
};
