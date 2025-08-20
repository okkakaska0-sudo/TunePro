#include "PluginProcessor.h"

// Minimal editor implementation - preserves full macOS compatibility
// Use this editor when MINIMAL_EDITOR is defined
#ifdef MINIMAL_EDITOR

class AutoTuneAudioProcessorEditor : public AudioProcessorEditor
{
public:
    AutoTuneAudioProcessorEditor(AutoTuneAudioProcessor& p)
        : AudioProcessorEditor(&p), audioProcessor(p)
    {
        setSize(800, 600);
        
        // Initialize minimal controls for full macOS compatibility
        speedSlider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        speedSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
        speedSlider.setRange(0.0, 100.0, 0.1);
        speedSlider.setValue(50.0);
        addAndMakeVisible(speedSlider);
        
        amountSlider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        amountSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
        amountSlider.setRange(0.0, 100.0, 0.1);
        amountSlider.setValue(50.0);
        addAndMakeVisible(amountSlider);
    }

    void paint(Graphics& g) override
    {
        g.fillAll(Colour(0xff1a1a1a));
        g.setColour(Colours::white);
        g.setFont(24.0f);
        g.drawText("AutoTune Plugin", 20, 20, getWidth() - 40, 40, Justification::centred);
        
        g.setFont(16.0f);
        g.drawText("Speed", 100, 100, 100, 20, Justification::centred);
        g.drawText("Amount", 300, 100, 100, 20, Justification::centred);
    }

    void resized() override
    {
        speedSlider.setBounds(50, 120, 100, 100);
        amountSlider.setBounds(250, 120, 100, 100);
    }

private:
    AutoTuneAudioProcessor& audioProcessor;
    Slider speedSlider;
    Slider amountSlider;
};

AudioProcessorEditor* AutoTuneAudioProcessor::createEditor()
{
    return new AutoTuneAudioProcessorEditor(*this);
}

#endif // MINIMAL_EDITOR
