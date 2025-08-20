#include "PluginEditor.h"
#include "Utils.h"

AutoTuneAudioProcessorEditor::AutoTuneAudioProcessorEditor(AutoTuneAudioProcessor& p)
    : AudioProcessorEditor(p), 
      audioProcessor(p),
      pitchHistory(pitchHistorySize, 0.0f)
{
    // Set custom look and feel
    setLookAndFeel(&lookAndFeel);
    
    // Setup all controls
    setupControls();
    
    // Set size
    setSize(800, 600);
    
    // Start timer for real-time updates
    startTimerHz(30); // 30 FPS
    
    // Initial layout
    setupLayout();
}

AutoTuneAudioProcessorEditor::~AutoTuneAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();
}

void AutoTuneAudioProcessorEditor::setupControls()
{
    // Speed Slider
    speedSlider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    speedSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    speedSlider.setRange(0.0, 100.0, 0.1);
    speedSlider.setValue(50.0);
    speedSlider.setSkewFactor(0.5); // Make lower values more accessible
    addAndMakeVisible(speedSlider);
    
    speedLabel.setText("Speed", dontSendNotification);
    speedLabel.setJustificationType(Justification::centred);
    speedLabel.setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(speedLabel);
    
    speedAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), Parameters::SPEED_ID, speedSlider);
    
    // Amount Slider
    amountSlider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    amountSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    amountSlider.setRange(0.0, 100.0, 0.1);
    amountSlider.setValue(50.0);
    addAndMakeVisible(amountSlider);
    
    amountLabel.setText("Amount", dontSendNotification);
    amountLabel.setJustificationType(Justification::centred);
    amountLabel.setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(amountLabel);
    
    amountAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), Parameters::AMOUNT_ID, amountSlider);
    
    // Mode Selector
    modeSelector.addItem("Classic", 1);
    modeSelector.addItem("Hard", 2);
    modeSelector.addItem("AI", 3);
    modeSelector.setSelectedId(1);
    addAndMakeVisible(modeSelector);
    
    modeLabel.setText("Mode", dontSendNotification);
    modeLabel.setJustificationType(Justification::centred);
    modeLabel.setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(modeLabel);
    
    modeAttachment = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), Parameters::MODE_ID, modeSelector);
    
    // Key Selector
    const char* keyNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    for (int i = 0; i < 12; ++i)
    {
        keySelector.addItem(keyNames[i], i + 1);
    }
    keySelector.setSelectedId(1); // C
    addAndMakeVisible(keySelector);
    
    keyLabel.setText("Key", dontSendNotification);
    keyLabel.setJustificationType(Justification::centred);
    keyLabel.setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(keyLabel);
    
    keyAttachment = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), Parameters::KEY_ID, keySelector);
    
    // Scale Selector
    scaleSelector.addItem("Major", 1);
    scaleSelector.addItem("Minor", 2);
    scaleSelector.addItem("Chromatic", 3);
    scaleSelector.setSelectedId(1); // Major
    addAndMakeVisible(scaleSelector);
    
    scaleLabel.setText("Scale", dontSendNotification);
    scaleLabel.setJustificationType(Justification::centred);
    scaleLabel.setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(scaleLabel);
    
    scaleAttachment = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), Parameters::SCALE_ID, scaleSelector);
    
    // Preset Controls
    savePresetButton.setButtonText("Save");
    savePresetButton.addListener(this);
    addAndMakeVisible(savePresetButton);
    
    loadPresetButton.setButtonText("Load");
    loadPresetButton.addListener(this);
    addAndMakeVisible(loadPresetButton);
    
    presetSelector.addItem("Default", 1);
    presetSelector.addItem("Vocal Classic", 2);
    presetSelector.addItem("Hard Tune", 3);
    presetSelector.addItem("AI Natural", 4);
    presetSelector.setSelectedId(1);
    addAndMakeVisible(presetSelector);
    
    updatePresetList();
}

void AutoTuneAudioProcessorEditor::setupLayout()
{
    auto bounds = getLocalBounds();
    
    // Define layout areas
    headerArea = bounds.removeFromTop(80);
    presetArea = bounds.removeFromBottom(60);
    controlsArea = bounds;
}

void AutoTuneAudioProcessorEditor::paint(Graphics& g)
{
    // Background gradient
    ColourGradient gradient(
        Colour(0xff1e1e2e), 0.0f, 0.0f,
        Colour(0xff181825), 0.0f, static_cast<float>(getHeight()),
        false
    );
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Draw sections
    drawHeader(g, headerArea);
    drawControls(g, controlsArea);
    drawPresetSection(g, presetArea);
}

void AutoTuneAudioProcessorEditor::drawHeader(Graphics& g, const Rectangle<int>& area)
{
    // Modern header with gradient
    ColourGradient headerGradient(
        Colour(0xff21262D), 0.0f, 0.0f,
        Colour(0xff161B22), 0.0f, static_cast<float>(area.getHeight()),
        false
    );
    g.setGradientFill(headerGradient);
    g.fillRoundedRectangle(area.toFloat(), 12.0f);
    
    // Modern glow border
    g.setColour(Colour(0xff00D4FF).withAlpha(0.3f));
    g.drawRoundedRectangle(area.toFloat().reduced(1.0f), 12.0f, 2.0f);
    
    // Title with modern typography
    g.setColour(Colour(0xff00D4FF));
    g.setFont(Font("Arial", 32.0f, Font::bold));
    g.drawText("AUTOTUNE PRO", area.reduced(30), Justification::centredLeft);
    
    // Subtitle
    g.setFont(Font("Arial", 13.0f, Font::plain));
    g.setColour(Colour(0xff7C3AED));
    g.drawText("AI-POWERED PITCH CORRECTION", area.reduced(30).translated(0, 25), Justification::centredLeft);
    
    // Status indicator
    g.setColour(Colour(0xff00FF88));
    g.fillEllipse(area.getRight() - 50, area.getCentreY() - 5, 10, 10);
    g.setFont(Font("Arial", 11.0f, Font::plain));
    g.setColour(Colours::white);
    g.drawText("ACTIVE", area.getRight() - 80, area.getCentreY() + 8, 60, 15, Justification::centred);
    
    // Draw level meters in header
    auto meterArea = area.reduced(30).removeFromRight(120).reduced(10);
    drawLevelMeters(g, meterArea);
}

void AutoTuneAudioProcessorEditor::drawControls(Graphics& g, const Rectangle<int>& area)
{
    auto bounds = area.reduced(20);
    
    // Main controls area
    auto mainControlsArea = bounds.removeFromTop(bounds.getHeight() * 0.6f);
    auto pitchDisplayArea = bounds;
    
    // Draw main controls background
    g.setColour(Colour(0xff2a2a3a));
    g.fillRoundedRectangle(mainControlsArea.toFloat(), 8.0f);
    
    // Draw pitch display
    drawPitchDisplay(g, pitchDisplayArea);
}

void AutoTuneAudioProcessorEditor::drawPresetSection(Graphics& g, const Rectangle<int>& area)
{
    // Preset section background
    g.setColour(Colour(0xff2d2d44));
    g.fillRoundedRectangle(area.toFloat(), 8.0f);
    
    // Preset label
    g.setColour(Colours::white);
    g.setFont(Font("Arial", 14.0f, Font::plain));
    g.drawText("Presets", area.reduced(20, 10), Justification::centredLeft);
}

void AutoTuneAudioProcessorEditor::drawPitchDisplay(Graphics& g, const Rectangle<int>& area)
{
    if (area.getWidth() < 10 || area.getHeight() < 10)
        return;
        
    // Background
    g.setColour(Colour(0xff1a1a2e));
    g.fillRoundedRectangle(area.toFloat(), 8.0f);
    
    // Border
    g.setColour(Colour(0xff3d3d5c));
    g.drawRoundedRectangle(area.toFloat(), 8.0f, 2.0f);
    
    // Title
    g.setColour(Colours::white);
    g.setFont(Font("Arial", 14.0f, Font::plain));
    g.drawText("Pitch Display", area.reduced(10).removeFromTop(20), Justification::centredLeft);
    
    // Draw pitch history
    auto displayArea = area.reduced(20, 30);
    if (displayArea.getWidth() > 0 && displayArea.getHeight() > 0)
    {
        Path pitchPath;
        bool firstPoint = true;
        
        for (int i = 0; i < pitchHistory.size(); ++i)
        {
            float x = displayArea.getX() + (float)i * displayArea.getWidth() / (float)pitchHistory.size();
            float y = displayArea.getBottom() - (pitchHistory[i] * 0.5f + 0.5f) * displayArea.getHeight();
            
            if (firstPoint)
            {
                pitchPath.startNewSubPath(x, y);
                firstPoint = false;
            }
            else
            {
                pitchPath.lineTo(x, y);
            }
        }
        
        // Draw pitch curve
        g.setColour(Colour(0xff4CAF50));
        g.strokePath(pitchPath, PathStrokeType(2.0f));
        
        // Draw center line (target pitch)
        g.setColour(Colour(0xff666666));
        float centerY = displayArea.getCentreY();
        g.drawLine(displayArea.getX(), centerY, displayArea.getRight(), centerY, 1.0f);
    }
}

void AutoTuneAudioProcessorEditor::drawLevelMeters(Graphics& g, const Rectangle<int>& area)
{
    auto meterWidth = 20;
    auto mutableArea = area;
    auto inputMeterArea = mutableArea.removeFromLeft(meterWidth);
    auto outputMeterArea = mutableArea.removeFromRight(meterWidth);
    
    // Input meter
    g.setColour(Colour(0xff333344));
    g.fillRect(inputMeterArea);
    
    auto inputHeight = static_cast<int>(currentInputLevel * inputMeterArea.getHeight());
    auto inputLevelArea = inputMeterArea.removeFromBottom(inputHeight);
    
    g.setColour(currentInputLevel > 0.8f ? Colours::red : Colours::green);
    g.fillRect(inputLevelArea);
    
    // Output meter
    g.setColour(Colour(0xff333344));
    g.fillRect(outputMeterArea);
    
    auto outputHeight = static_cast<int>(currentOutputLevel * outputMeterArea.getHeight());
    auto outputLevelArea = outputMeterArea.removeFromBottom(outputHeight);
    
    g.setColour(currentOutputLevel > 0.8f ? Colours::red : Colours::blue);
    g.fillRect(outputLevelArea);
    
    // Labels
    g.setColour(Colours::white);
    g.setFont(10.0f);
    g.drawText("IN", inputMeterArea.withY(area.getBottom() + 5).withHeight(15), Justification::centred);
    g.drawText("OUT", outputMeterArea.withY(area.getBottom() + 5).withHeight(15), Justification::centred);
}

void AutoTuneAudioProcessorEditor::resized()
{
    setupLayout();
    
    auto bounds = getLocalBounds();
    
    // Skip header and preset areas for control layout
    auto controlsBounds = bounds.reduced(40, 100);
    controlsBounds.removeFromBottom(60);
    
    // Main rotary controls
    auto rotaryArea = controlsBounds.removeFromTop(200);
    auto sliderWidth = rotaryArea.getWidth() / 2 - 20;
    
    auto speedArea = rotaryArea.removeFromLeft(sliderWidth);
    speedSlider.setBounds(speedArea.reduced(20, 10));
    speedLabel.setBounds(speedArea.removeFromBottom(20));
    
    rotaryArea.removeFromLeft(40); // Spacing
    
    auto amountArea = rotaryArea;
    amountSlider.setBounds(amountArea.reduced(20, 10));
    amountLabel.setBounds(amountArea.removeFromBottom(20));
    
    // Selector controls
    auto selectorArea = controlsBounds.removeFromTop(80);
    auto selectorWidth = selectorArea.getWidth() / 3 - 20;
    
    auto modeArea = selectorArea.removeFromLeft(selectorWidth);
    modeLabel.setBounds(modeArea.removeFromTop(20));
    modeSelector.setBounds(modeArea.reduced(10));
    
    selectorArea.removeFromLeft(30); // Spacing
    
    auto keyArea = selectorArea.removeFromLeft(selectorWidth);
    keyLabel.setBounds(keyArea.removeFromTop(20));
    keySelector.setBounds(keyArea.reduced(10));
    
    selectorArea.removeFromLeft(30); // Spacing
    
    auto scaleArea = selectorArea;
    scaleLabel.setBounds(scaleArea.removeFromTop(20));
    scaleSelector.setBounds(scaleArea.reduced(10));
    
    // Preset controls at bottom
    auto presetBounds = bounds.removeFromBottom(60).reduced(20, 10);
    auto buttonWidth = 80;
    
    savePresetButton.setBounds(presetBounds.removeFromLeft(buttonWidth));
    presetBounds.removeFromLeft(10);
    loadPresetButton.setBounds(presetBounds.removeFromLeft(buttonWidth));
    presetBounds.removeFromLeft(20);
    presetSelector.setBounds(presetBounds);
}

void AutoTuneAudioProcessorEditor::timerCallback()
{
    // Update level meters (simplified - in real implementation would get from processor)
    currentInputLevel = 0.3f + 0.2f * std::sin(Time::getMillisecondCounter() * 0.01f);
    currentOutputLevel = 0.25f + 0.15f * std::cos(Time::getMillisecondCounter() * 0.008f);
    
    // Update pitch history (placeholder data)
    for (int i = 0; i < pitchHistory.size() - 1; ++i)
    {
        pitchHistory[i] = pitchHistory[i + 1];
    }
    pitchHistory.back() = 0.1f * std::sin(Time::getMillisecondCounter() * 0.005f);
    
    repaint();
}

void AutoTuneAudioProcessorEditor::buttonClicked(Button* button)
{
    if (button == &savePresetButton)
    {
        // Save current settings as preset
        audioProcessor.getPresetManager().savePreset("User Preset");
        updatePresetList();
    }
    else if (button == &loadPresetButton)
    {
        // Load selected preset
        int selectedId = presetSelector.getSelectedId();
        if (selectedId > 0)
        {
            audioProcessor.getPresetManager().loadPreset(selectedId - 1);
        }
    }
}

void AutoTuneAudioProcessorEditor::updatePresetList()
{
    // Update preset selector with available presets
    presetSelector.clear();
    
    auto& presetManager = audioProcessor.getPresetManager();
    auto presetNames = presetManager.getPresetNames();
    
    for (int i = 0; i < presetNames.size(); ++i)
    {
        presetSelector.addItem(presetNames[i], i + 1);
    }
    
    if (presetSelector.getNumItems() > 0)
        presetSelector.setSelectedId(1);
}
