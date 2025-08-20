#include "ModeSelector.h"
#include "LookAndFeel.h"

ModeSelector::ModeSelector()
{
    // Setup title label
    titleLabel.setText("Processing Mode", dontSendNotification);
    titleLabel.setFont(Font(14.0f, Font::bold));
    titleLabel.setJustificationType(Justification::centred);
    titleLabel.setColour(Label::textColourId, ProAutoTuneLookAndFeel::Colors::textGold);
    addAndMakeVisible(titleLabel);
    
    // Setup mode combo box
    modeComboBox.addItem("Classic Mode", static_cast<int>(Parameters::Mode::Classic) + 1);
    modeComboBox.addItem("Hard Mode", static_cast<int>(Parameters::Mode::Hard) + 1);
    modeComboBox.addItem("AI Mode", static_cast<int>(Parameters::Mode::AI) + 1);
    
    modeComboBox.setSelectedId(static_cast<int>(Parameters::Mode::Classic) + 1, dontSendNotification);
    modeComboBox.addListener(this);
    addAndMakeVisible(modeComboBox);
    
    // Setup description label
    descriptionLabel.setFont(Font(11.0f));
    descriptionLabel.setJustificationType(Justification::centred);
    descriptionLabel.setColour(Label::textColourId, ProAutoTuneLookAndFeel::Colors::textWhite);
    addAndMakeVisible(descriptionLabel);
    
    // Setup mode styles
    setupModeStyles();
    
    // Set default descriptions
    StringArray defaultDescriptions;
    defaultDescriptions.add("Natural vocal correction with smooth transitions");
    defaultDescriptions.add("Aggressive T-Pain style pitch snapping");
    defaultDescriptions.add("AI-powered intelligent pitch correction");
    setModeDescriptions(defaultDescriptions);
    
    updateModeDisplay();
}

ModeSelector::~ModeSelector()
{
    modeComboBox.removeListener(this);
}

void ModeSelector::paint(Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw background panel
    g.setColour(ProAutoTuneLookAndFeel::Colors::backgroundLight);
    g.fillRoundedRectangle(bounds, 5.0f);
    
    // Draw mode-specific accent border
    auto modeStyle = modeStyles.find(currentMode);
    if (modeStyle != modeStyles.end())
    {
        g.setColour(modeStyle->second.accentColour.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds.reduced(1.0f), 4.0f, 2.0f);
        
        // Draw mode icon if available
        auto iconBounds = bounds.removeFromTop(30.0f).reduced(5.0f);
        drawModeIcon(g, iconBounds, currentMode);
    }
}

void ModeSelector::resized()
{
    auto bounds = getLocalBounds();
    
    // Title at top
    titleLabel.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(5); // Spacing
    
    // Icon area (handled in paint)
    bounds.removeFromTop(30);
    bounds.removeFromTop(5);
    
    // Combo box in middle
    modeComboBox.setBounds(bounds.removeFromTop(25).reduced(5, 0));
    bounds.removeFromTop(5);
    
    // Description at bottom
    if (showDescription && !descriptionLabel.getText().isEmpty())
    {
        descriptionLabel.setBounds(bounds.reduced(5, 0));
    }
}

void ModeSelector::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &modeComboBox)
    {
        int selectedId = modeComboBox.getSelectedId();
        if (selectedId > 0)
        {
            auto newMode = static_cast<Parameters::Mode>(selectedId - 1);
            setCurrentMode(newMode);
            
            if (onModeChanged)
                onModeChanged(newMode);
        }
    }
}

void ModeSelector::setCurrentMode(Parameters::Mode mode)
{
    if (currentMode != mode)
    {
        currentMode = mode;
        modeComboBox.setSelectedId(static_cast<int>(mode) + 1, dontSendNotification);
        updateModeDisplay();
        repaint();
    }
}

void ModeSelector::setModeDescriptions(const StringArray& descriptions)
{
    modeDescriptions = descriptions;
    updateModeDisplay();
}

void ModeSelector::setupModeStyles()
{
    // Classic mode - warm gold/amber colors
    modeStyles.emplace(Parameters::Mode::Classic, 
        ModeStyle(Colour(0xffcd853f), Colour(0xffffd700), "classic_icon"));
    
    // Hard mode - aggressive red/orange colors  
    modeStyles.emplace(Parameters::Mode::Hard,
        ModeStyle(Colour(0xffff4500), Colour(0xffff6347), "hard_icon"));
    
    // AI mode - futuristic blue/cyan colors
    modeStyles.emplace(Parameters::Mode::AI,
        ModeStyle(Colour(0xff0080ff), Colour(0xff00ffff), "ai_icon"));
}

void ModeSelector::updateModeDisplay()
{
    // Update description text
    int modeIndex = static_cast<int>(currentMode);
    if (modeIndex < modeDescriptions.size())
    {
        descriptionLabel.setText(modeDescriptions[modeIndex], dontSendNotification);
    }
    
    // Update colors based on current mode
    auto modeStyle = modeStyles.find(currentMode);
    if (modeStyle != modeStyles.end())
    {
        modeComboBox.setColour(ComboBox::backgroundColourId, 
                              modeStyle->second.primaryColour.withAlpha(0.1f));
        modeComboBox.setColour(ComboBox::outlineColourId, 
                              modeStyle->second.accentColour);
    }
    
    repaint();
}

void ModeSelector::drawModeIcon(Graphics& g, Rectangle<float> iconBounds, Parameters::Mode mode)
{
    g.setColour(ProAutoTuneLookAndFeel::Colors::textWhite.withAlpha(0.8f));
    
    switch (mode)
    {
        case Parameters::Mode::Classic:
        {
            // Draw sine wave icon for classic mode
            Path wavePath;
            float width = iconBounds.getWidth();
            float height = iconBounds.getHeight();
            float centerY = iconBounds.getCentreY();
            
            wavePath.startNewSubPath(iconBounds.getX(), centerY);
            
            for (float x = 0; x <= width; x += 2.0f)
            {
                float angle = (x / width) * MathConstants<float>::twoPi * 2.0f;
                float y = centerY + std::sin(angle) * height * 0.3f;
                wavePath.lineTo(iconBounds.getX() + x, y);
            }
            
            g.strokePath(wavePath, PathStrokeType(2.0f, PathStrokeType::curved));
            break;
        }
        
        case Parameters::Mode::Hard:
        {
            // Draw square wave icon for hard mode
            Path squarePath;
            float width = iconBounds.getWidth();
            float height = iconBounds.getHeight();
            float centerY = iconBounds.getCentreY();
            
            squarePath.startNewSubPath(iconBounds.getX(), centerY + height * 0.2f);
            squarePath.lineTo(iconBounds.getX() + width * 0.25f, centerY + height * 0.2f);
            squarePath.lineTo(iconBounds.getX() + width * 0.25f, centerY - height * 0.2f);
            squarePath.lineTo(iconBounds.getX() + width * 0.75f, centerY - height * 0.2f);
            squarePath.lineTo(iconBounds.getX() + width * 0.75f, centerY + height * 0.2f);
            squarePath.lineTo(iconBounds.getRight(), centerY + height * 0.2f);
            
            g.strokePath(squarePath, PathStrokeType(2.0f));
            break;
        }
        
        case Parameters::Mode::AI:
        {
            // Draw AI neural network icon
            std::vector<Point<float>> nodes = {
                {iconBounds.getX() + width * 0.2f, iconBounds.getY() + height * 0.3f},
                {iconBounds.getX() + width * 0.2f, iconBounds.getY() + height * 0.7f},
                {iconBounds.getX() + width * 0.5f, iconBounds.getCentreY()},
                {iconBounds.getX() + width * 0.8f, iconBounds.getY() + height * 0.3f},
                {iconBounds.getX() + width * 0.8f, iconBounds.getY() + height * 0.7f}
            };
            
            // Draw connections
            g.setColour(ProAutoTuneLookAndFeel::Colors::textWhite.withAlpha(0.4f));
            for (int i = 0; i < 2; ++i)
            {
                for (int j = 2; j < 5; ++j)
                {
                    g.drawLine(Line<float>(nodes[i], nodes[j]), 1.0f);
                }
            }
            
            // Draw nodes
            g.setColour(ProAutoTuneLookAndFeel::Colors::textWhite.withAlpha(0.8f));
            for (const auto& node : nodes)
            {
                g.fillEllipse(Rectangle<float>(4.0f, 4.0f).withCentre(node));
            }
            break;
        }
    }
}

// ModeVisualizerComponent implementation

ModeVisualizerComponent::ModeVisualizerComponent()
{
    waveformData.resize(256, 0.0f);
    spectrumData.resize(128, 0.0f);
    startTimer(30); // 30ms refresh rate
}

ModeVisualizerComponent::~ModeVisualizerComponent()
{
}

void ModeVisualizerComponent::paint(Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw background
    g.setColour(Colour(0xff0a0a0a));
    g.fillRoundedRectangle(bounds, 3.0f);
    
    // Draw mode-specific visualization
    switch (currentMode)
    {
        case Parameters::Mode::Classic:
            drawClassicVisualization(g, bounds);
            break;
            
        case Parameters::Mode::Hard:
            drawHardVisualization(g, bounds);
            break;
            
        case Parameters::Mode::AI:
            drawAIVisualization(g, bounds);
            break;
    }
    
    // Draw border
    g.setColour(ProAutoTuneLookAndFeel::Colors::metalDark);
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
}

void ModeVisualizerComponent::timerCallback()
{
    animationPhase += 0.1f;
    if (animationPhase > MathConstants<float>::twoPi)
        animationPhase -= MathConstants<float>::twoPi;
    
    updateWaveformData();
    updateSpectrumData();
    repaint();
}

void ModeVisualizerComponent::setMode(Parameters::Mode mode)
{
    currentMode = mode;
    repaint();
}

void ModeVisualizerComponent::setInputSignal(const float* buffer, int numSamples)
{
    if (buffer && numSamples > 0)
    {
        // Update waveform data with input signal
        int step = jmax(1, numSamples / static_cast<int>(waveformData.size()));
        for (int i = 0; i < waveformData.size() && i * step < numSamples; ++i)
        {
            waveformData[i] = buffer[i * step] * processingLevel;
        }
    }
}

void ModeVisualizerComponent::drawClassicVisualization(Graphics& g, Rectangle<float> bounds)
{
    // Draw smooth waveform
    g.setColour(Colour(0xffcd853f).withAlpha(0.7f));
    
    Path waveformPath;
    float centerY = bounds.getCentreY();
    bool firstPoint = true;
    
    for (int i = 0; i < waveformData.size(); ++i)
    {
        float x = bounds.getX() + (i * bounds.getWidth()) / waveformData.size();
        float y = centerY + waveformData[i] * bounds.getHeight() * 0.3f;
        
        if (firstPoint)
        {
            waveformPath.startNewSubPath(x, y);
            firstPoint = false;
        }
        else
        {
            waveformPath.lineTo(x, y);
        }
    }
    
    g.strokePath(waveformPath, PathStrokeType(2.0f, PathStrokeType::curved));
    
    // Add gentle glow effect
    g.setColour(Colour(0xffcd853f).withAlpha(0.2f));
    g.strokePath(waveformPath, PathStrokeType(4.0f, PathStrokeType::curved));
}

void ModeVisualizerComponent::drawHardVisualization(Graphics& g, Rectangle<float> bounds)
{
    // Draw quantized waveform with sharp edges
    g.setColour(Colour(0xffff4500).withAlpha(0.8f));
    
    Path hardPath;
    float centerY = bounds.getCentreY();
    float lastQuantizedValue = 0.0f;
    
    for (int i = 0; i < waveformData.size(); ++i)
    {
        float x = bounds.getX() + (i * bounds.getWidth()) / waveformData.size();
        
        // Quantize the waveform value
        float quantizedValue = waveformData[i] > 0 ? 0.5f : -0.5f;
        float y = centerY + quantizedValue * bounds.getHeight() * 0.4f;
        
        if (i == 0)
        {
            hardPath.startNewSubPath(x, y);
        }
        else
        {
            // Create sharp transitions
            if (quantizedValue != lastQuantizedValue)
            {
                hardPath.lineTo(x, centerY + lastQuantizedValue * bounds.getHeight() * 0.4f);
            }
            hardPath.lineTo(x, y);
        }
        
        lastQuantizedValue = quantizedValue;
    }
    
    g.strokePath(hardPath, PathStrokeType(2.0f));
    
    // Add aggressive glow
    g.setColour(Colour(0xffff4500).withAlpha(0.3f));
    g.strokePath(hardPath, PathStrokeType(5.0f));
}

void ModeVisualizerComponent::drawAIVisualization(Graphics& g, Rectangle<float> bounds)
{
    // Draw spectrum analysis visualization
    g.setColour(Colour(0xff0080ff).withAlpha(0.6f));
    
    float barWidth = bounds.getWidth() / spectrumData.size();
    
    for (int i = 0; i < spectrumData.size(); ++i)
    {
        float x = bounds.getX() + i * barWidth;
        float height = spectrumData[i] * bounds.getHeight() * 0.8f;
        float y = bounds.getBottom() - height;
        
        Rectangle<float> bar(x, y, barWidth - 1, height);
        
        // Create gradient for each bar
        ColourGradient barGradient(
            Colour(0xff00ffff).withAlpha(0.8f), x, bounds.getBottom(),
            Colour(0xff0080ff).withAlpha(0.3f), x, y, false
        );
        
        g.setGradientFill(barGradient);
        g.fillRect(bar);
    }
    
    // Add AI processing overlay animation
    g.setColour(Colour(0xff00ffff).withAlpha(0.3f + 0.2f * std::sin(animationPhase * 2)));
    for (int i = 0; i < 3; ++i)
    {
        float y = bounds.getY() + (i + 1) * bounds.getHeight() / 4;
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
    }
}

void ModeVisualizerComponent::updateWaveformData()
{
    // Simulate waveform evolution based on processing level
    for (int i = 1; i < waveformData.size(); ++i)
    {
        waveformData[i - 1] = waveformData[i];
    }
    
    // Add new data point
    float newValue = std::sin(animationPhase + waveformData.size() * 0.1f) * processingLevel;
    waveformData.back() = newValue;
}

void ModeVisualizerComponent::updateSpectrumData()
{
    // Simulate spectrum data evolution
    for (int i = 0; i < spectrumData.size(); ++i)
    {
        float frequency = static_cast<float>(i) / spectrumData.size();
        float amplitude = std::abs(std::sin(animationPhase + frequency * 10)) * processingLevel;
        
        // Smooth the data
        spectrumData[i] = spectrumData[i] * 0.7f + amplitude * 0.3f;
    }
}
