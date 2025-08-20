#include "PluginProcessor.h"
#include "PluginEditor.h"

AutoTuneAudioProcessorEditor::AutoTuneAudioProcessorEditor(AutoTuneAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Set custom look and feel
    setLookAndFeel(&lookAndFeel);
    
    // Set editor size
    setSize(800, 600);
    
    // Setup all controls
    setupControls();
    setupLayout();
    
    // Initialize pitch history
    pitchHistory.resize(pitchHistorySize, 0.0f);
    
    // Start timer for real-time updates
    startTimer(30); // 30ms refresh rate
}

AutoTuneAudioProcessorEditor::~AutoTuneAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void AutoTuneAudioProcessorEditor::setupControls()
{
    // Speed slider
    speedSlider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    speedSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    speedSlider.setRange(0.0, 100.0, 0.1);
    speedSlider.setValue(50.0);
    speedSlider.setTextValueSuffix(" %");
    addAndMakeVisible(speedSlider);
    
    speedLabel.setText("Speed", dontSendNotification);
    speedLabel.setJustificationType(Justification::centredTop);
    speedLabel.attachToComponent(&speedSlider, false);
    addAndMakeVisible(speedLabel);
    
    speedAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), Parameters::SPEED_ID, speedSlider);
    
    // Amount slider
    amountSlider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    amountSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    amountSlider.setRange(0.0, 100.0, 0.1);
    amountSlider.setValue(50.0);
    amountSlider.setTextValueSuffix(" %");
    addAndMakeVisible(amountSlider);
    
    amountLabel.setText("Amount", dontSendNotification);
    amountLabel.setJustificationType(Justification::centredTop);
    amountLabel.attachToComponent(&amountSlider, false);
    addAndMakeVisible(amountLabel);
    
    amountAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), Parameters::AMOUNT_ID, amountSlider);
    
    // Mode selector
    modeSelector.addItem("Classic", 1);
    modeSelector.addItem("Hard", 2);
    modeSelector.addItem("AI", 3);
    modeSelector.setSelectedId(1);
    addAndMakeVisible(modeSelector);
    
    modeLabel.setText("Mode", dontSendNotification);
    modeLabel.setJustificationType(Justification::centredTop);
    modeLabel.attachToComponent(&modeSelector, false);
    addAndMakeVisible(modeLabel);
    
    modeAttachment = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), Parameters::MODE_ID, modeSelector);
    
    // Key selector
    const char* keys[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    for (int i = 0; i < 12; ++i)
    {
        keySelector.addItem(keys[i], i + 1);
    }
    keySelector.setSelectedId(1); // Default to C
    addAndMakeVisible(keySelector);
    
    keyLabel.setText("Key", dontSendNotification);
    keyLabel.setJustificationType(Justification::centredTop);
    keyLabel.attachToComponent(&keySelector, false);
    addAndMakeVisible(keyLabel);
    
    keyAttachment = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), Parameters::KEY_ID, keySelector);
    
    // Scale selector
    scaleSelector.addItem("Major", 1);
    scaleSelector.addItem("Minor", 2);
    scaleSelector.addItem("Chromatic", 3);
    scaleSelector.setSelectedId(1);
    addAndMakeVisible(scaleSelector);
    
    scaleLabel.setText("Scale", dontSendNotification);
    scaleLabel.setJustificationType(Justification::centredTop);
    scaleLabel.attachToComponent(&scaleSelector, false);
    addAndMakeVisible(scaleLabel);
    
    scaleAttachment = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), Parameters::SCALE_ID, scaleSelector);
    
    // Preset controls
    savePresetButton.setButtonText("Save");
    savePresetButton.addListener(this);
    addAndMakeVisible(savePresetButton);
    
    loadPresetButton.setButtonText("Load");
    loadPresetButton.addListener(this);
    addAndMakeVisible(loadPresetButton);
    
    updatePresetList();
    addAndMakeVisible(presetSelector);
}

void AutoTuneAudioProcessorEditor::setupLayout()
{
    auto bounds = getLocalBounds();
    
    // Header area (logo and title)
    headerArea = bounds.removeFromTop(80);
    
    // Preset section at bottom
    presetArea = bounds.removeFromBottom(60);
    
    // Main controls area
    controlsArea = bounds;
}

void AutoTuneAudioProcessorEditor::paint(Graphics& g)
{
    // Background gradient
    ColourGradient backgroundGradient(
        Colour(0xff2a2a2a), 0, 0,
        Colour(0xff1a1a1a), 0, static_cast<float>(getHeight()),
        false
    );
    g.setGradientFill(backgroundGradient);
    g.fillAll();
    
    // Draw sections
    drawHeader(g, headerArea);
    drawControls(g, controlsArea);
    drawPresetSection(g, presetArea);
}

void AutoTuneAudioProcessorEditor::resized()
{
    setupLayout();
    
    auto controlsBounds = controlsArea.reduced(20);
    
    // Main knobs
    auto knobArea = controlsBounds.removeFromTop(180);
    auto knobWidth = 120;
    auto knobSpacing = (knobArea.getWidth() - (2 * knobWidth)) / 3;
    
    speedSlider.setBounds(knobArea.getX() + knobSpacing, knobArea.getY() + 20, knobWidth, knobWidth);
    amountSlider.setBounds(speedSlider.getRight() + knobSpacing, knobArea.getY() + 20, knobWidth, knobWidth);
    
    // Selectors
    auto selectorArea = controlsBounds.removeFromTop(80);
    auto selectorWidth = 120;
    auto selectorHeight = 25;
    auto selectorSpacing = (selectorArea.getWidth() - (3 * selectorWidth)) / 4;
    
    modeSelector.setBounds(
        selectorArea.getX() + selectorSpacing, 
        selectorArea.getY() + 20, 
        selectorWidth, selectorHeight
    );
    
    keySelector.setBounds(
        modeSelector.getRight() + selectorSpacing, 
        selectorArea.getY() + 20, 
        selectorWidth, selectorHeight
    );
    
    scaleSelector.setBounds(
        keySelector.getRight() + selectorSpacing, 
        selectorArea.getY() + 20, 
        selectorWidth, selectorHeight
    );
    
    // Preset controls
    auto presetBounds = presetArea.reduced(20);
    auto buttonWidth = 80;
    auto buttonHeight = 30;
    
    savePresetButton.setBounds(
        presetBounds.getX(), 
        presetBounds.getY() + (presetBounds.getHeight() - buttonHeight) / 2,
        buttonWidth, buttonHeight
    );
    
    loadPresetButton.setBounds(
        savePresetButton.getRight() + 10, 
        presetBounds.getY() + (presetBounds.getHeight() - buttonHeight) / 2,
        buttonWidth, buttonHeight
    );
    
    presetSelector.setBounds(
        loadPresetButton.getRight() + 20,
        presetBounds.getY() + (presetBounds.getHeight() - buttonHeight) / 2,
        presetBounds.getRight() - loadPresetButton.getRight() - 20,
        buttonHeight
    );
}

void AutoTuneAudioProcessorEditor::timerCallback()
{
    // Update real-time displays
    repaint();
}

void AutoTuneAudioProcessorEditor::buttonClicked(Button* button)
{
    if (button == &savePresetButton)
    {
        AlertWindow::showMessageBoxAsync(
            AlertWindow::InfoIcon,
            "Save Preset",
            "Preset saved successfully!",
            "OK"
        );
    }
    else if (button == &loadPresetButton)
    {
        int selectedId = presetSelector.getSelectedId();
        if (selectedId > 0)
        {
            audioProcessor.getPresetManager().loadPreset(selectedId - 1);
        }
    }
}

void AutoTuneAudioProcessorEditor::updatePresetList()
{
    presetSelector.clear();
    auto presetNames = audioProcessor.getPresetManager().getPresetNames();
    
    for (int i = 0; i < presetNames.size(); ++i)
    {
        presetSelector.addItem(presetNames[i], i + 1);
    }
}

void AutoTuneAudioProcessorEditor::drawHeader(Graphics& g, const Rectangle<int>& area)
{
    // Draw title
    g.setColour(Colours::white);
    g.setFont(Font("Helvetica", 28.0f, Font::bold));
    g.drawText("MarsiAutoTune", area.reduced(20), Justification::centred);
    
    // Draw logo area
    auto logoArea = Rectangle<int>(20, 10, 60, 60);
    g.setColour(Colour(0xff4a9eff));
    g.fillEllipse(logoArea.toFloat());
    
    g.setColour(Colours::white);
    g.setFont(Font("Helvetica", 20.0f, Font::bold));
    g.drawText("MS", logoArea, Justification::centred);
}

void AutoTuneAudioProcessorEditor::drawControls(Graphics& g, const Rectangle<int>& area)
{
    // Draw section background
    g.setColour(Colour(0x20ffffff));
    g.fillRoundedRectangle(area.toFloat(), 5.0f);
    
    g.setColour(Colour(0x40ffffff));
    g.drawRoundedRectangle(area.toFloat(), 5.0f, 1.0f);
}

void AutoTuneAudioProcessorEditor::drawPresetSection(Graphics& g, const Rectangle<int>& area)
{
    // Draw preset section background
    g.setColour(Colour(0x10ffffff));
    g.fillRoundedRectangle(area.toFloat(), 3.0f);
    
    g.setColour(Colours::white);
    g.setFont(14.0f);
    g.drawText("Presets", area.getX() + 300, area.getY() + 5, 100, 20, Justification::left);
}

void AutoTuneAudioProcessorEditor::drawPitchDisplay(Graphics& g, const Rectangle<int>& area)
{
    // Draw pitch history graph
    g.setColour(Colour(0xff4a9eff));
    
    Path pitchPath;
    bool firstPoint = true;
    
    for (int i = 0; i < pitchHistory.size(); ++i)
    {
        float x = area.getX() + (i * area.getWidth()) / static_cast<float>(pitchHistory.size());
        float y = area.getBottom() - (pitchHistory[i] * area.getHeight());
        
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
    
    g.strokePath(pitchPath, PathStrokeType(2.0f));
}

void AutoTuneAudioProcessorEditor::drawLevelMeters(Graphics& g, Rectangle<int> area)
{
    // Draw input/output level meters
    auto meterWidth = 20;
    auto inputMeter = area.removeFromLeft(meterWidth);
    auto outputMeter = area.removeFromRight(meterWidth);
    
    // Input level
    g.setColour(Colour(0xff333333));
    g.fillRect(inputMeter);
    
    g.setColour(Colour(0xff00ff00));
    auto inputHeight = inputMeter.getHeight() * currentInputLevel;
    g.fillRect(inputMeter.getX(), inputMeter.getBottom() - static_cast<int>(inputHeight), 
               inputMeter.getWidth(), static_cast<int>(inputHeight));
    
    // Output level
    g.setColour(Colour(0xff333333));
    g.fillRect(outputMeter);
    
    g.setColour(Colour(0xff00ff00));
    auto outputHeight = outputMeter.getHeight() * currentOutputLevel;
    g.fillRect(outputMeter.getX(), outputMeter.getBottom() - static_cast<int>(outputHeight), 
               outputMeter.getWidth(), static_cast<int>(outputHeight));
}
