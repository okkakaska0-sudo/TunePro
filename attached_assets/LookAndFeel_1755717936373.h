#pragma once

#include "JuceHeader.h"

class ProAutoTuneLookAndFeel : public LookAndFeel_V4
{
public:
    ProAutoTuneLookAndFeel();
    ~ProAutoTuneLookAndFeel() override;

    // Slider customization
    void drawRotarySlider(Graphics& g, int x, int y, int width, int height,
                         float sliderPosProportional, float rotaryStartAngle,
                         float rotaryEndAngle, Slider& slider) override;
    
    void drawLinearSlider(Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         const Slider::SliderStyle style, Slider& slider) override;
    
    Label* createSliderTextBox(Slider& slider) override;
    
    // ComboBox customization
    void drawComboBox(Graphics& g, int width, int height, bool isButtonDown,
                     int buttonX, int buttonY, int buttonW, int buttonH,
                     ComboBox& comboBox) override;
    
    void positionComboBoxText(ComboBox& comboBox, Label& labelToPosition) override;
    
    // Button customization
    void drawButtonBackground(Graphics& g, Button& button,
                             const Colour& backgroundColour,
                             bool shouldDrawButtonAsHighlighted,
                             bool shouldDrawButtonAsDown) override;
    
    void drawButtonText(Graphics& g, TextButton& button,
                       bool shouldDrawButtonAsHighlighted,
                       bool shouldDrawButtonAsDown) override;
    
    // Label customization
    void drawLabel(Graphics& g, Label& label) override;
    
    // Popup menu customization
    void drawPopupMenuBackground(Graphics& g, int width, int height) override;
    void drawPopupMenuItem(Graphics& g, const Rectangle<int>& area,
                          bool isSeparator, bool isActive, bool isHighlighted,
                          bool isTicked, bool hasSubMenu, const String& text,
                          const String& shortcutKeyText,
                          const Drawable* icon, const Colour* textColour) override;

    // Color scheme
    struct Colors
    {
        static const Colour background;
        static const Colour surface;
        static const Colour surfaceVariant;
        static const Colour primary;
        static const Colour primaryVariant;
        static const Colour secondary;
        static const Colour accent;
        static const Colour textPrimary;
        static const Colour textSecondary;
        static const Colour textDisabled;
        static const Colour border;
        static const Colour borderFocus;
        static const Colour highlight;
        static const Colour shadow;
    };

private:
    // Helper methods
    void drawGlowEffect(Graphics& g, const Rectangle<float>& bounds,
                       Colour glowColor, float intensity = 1.0f);
    
    void drawGradientBackground(Graphics& g, const Rectangle<int>& bounds,
                               Colour topColor, Colour bottomColor);
    
    void drawRoundedPath(Graphics& g, const Path& path,
                        Colour fillColor, Colour strokeColor,
                        float strokeWidth = 1.0f);
    
    // Custom fonts
    Font getTitleFont() const;
    Font getControlFont() const;
    Font getSmallFont() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProAutoTuneLookAndFeel)
};
