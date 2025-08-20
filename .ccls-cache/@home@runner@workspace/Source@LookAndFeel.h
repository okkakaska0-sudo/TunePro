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

    // Button customization
    void drawButtonBackground(Graphics& g, Button& button, const Colour& backgroundColour,
                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    void drawButtonText(Graphics& g, TextButton& button, bool shouldDrawButtonAsHighlighted,
                       bool shouldDrawButtonAsDown) override;

    // ComboBox customization
    void drawComboBox(Graphics& g, int width, int height, bool isButtonDown,
                     int buttonX, int buttonY, int buttonW, int buttonH,
                     ComboBox& box) override;

    // Label customization
    void drawLabel(Graphics& g, Label& label) override;

    // Custom drawing methods
    void drawVintageKnob(Graphics& g, Rectangle<float> bounds, float sliderPos,
                        float rotaryStartAngle, float rotaryEndAngle, 
                        const Colour& knobColour);
    
    void drawMetalFrame(Graphics& g, Rectangle<float> bounds, float cornerSize = 5.0f);
    void drawGlossyButton(Graphics& g, Rectangle<float> bounds, bool isPressed, 
                         const Colour& baseColour);
    void drawVUMeter(Graphics& g, Rectangle<float> bounds, float level, 
                    bool isInput = true);

    // Color scheme
    struct Colors
    {
        static const Colour metalDark;
        static const Colour metalLight;
        static const Colour knobBrass;
        static const Colour knobBrassLight;
        static const Colour textWhite;
        static const Colour textGold;
        static const Colour ledGreen;
        static const Colour ledRed;
        static const Colour ledBlue;
        static const Colour background;
        static const Colour backgroundLight;
    };

private:
    // Cached graphics resources
    std::unique_ptr<Drawable> logoDrawable;
    std::unique_ptr<Drawable> knobBackgroundDrawable;
    std::unique_ptr<Drawable> knobPointerDrawable;
    
    // Helper methods
    void createGradientFill(ColourGradient& gradient, Rectangle<float> bounds, 
                           const Colour& color1, const Colour& color2, bool vertical = true);
    void drawInnerShadow(Graphics& g, Rectangle<float> bounds, float cornerSize = 0.0f);
    void drawOuterGlow(Graphics& g, Rectangle<float> bounds, const Colour& glowColour, 
                      float glowRadius = 3.0f);
    Path createRoundedPath(Rectangle<float> bounds, float cornerSize);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProAutoTuneLookAndFeel)
};
