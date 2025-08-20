#include "LookAndFeel.h"

// Color definitions
const Colour ProAutoTuneLookAndFeel::Colors::metalDark = Colour(0xff2a2a2a);
const Colour ProAutoTuneLookAndFeel::Colors::metalLight = Colour(0xff4a4a4a);
const Colour ProAutoTuneLookAndFeel::Colors::knobBrass = Colour(0xffcd853f);
const Colour ProAutoTuneLookAndFeel::Colors::knobBrassLight = Colour(0xffdaa520);
const Colour ProAutoTuneLookAndFeel::Colors::textWhite = Colour(0xfff0f0f0);
const Colour ProAutoTuneLookAndFeel::Colors::textGold = Colour(0xffffd700);
const Colour ProAutoTuneLookAndFeel::Colors::ledGreen = Colour(0xff00ff00);
const Colour ProAutoTuneLookAndFeel::Colors::ledRed = Colour(0xffff0000);
const Colour ProAutoTuneLookAndFeel::Colors::ledBlue = Colour(0xff0080ff);
const Colour ProAutoTuneLookAndFeel::Colors::background = Colour(0xff1a1a1a);
const Colour ProAutoTuneLookAndFeel::Colors::backgroundLight = Colour(0xff2a2a2a);

ProAutoTuneLookAndFeel::ProAutoTuneLookAndFeel()
{
    // Set default fonts
    setDefaultSansSerifTypefaceName("Helvetica");
    
    // Load SVG assets
    logoDrawable = Drawable::createFromImageData(BinaryData::logo_svg, BinaryData::logo_svgSize);
    knobBackgroundDrawable = Drawable::createFromImageData(BinaryData::knob_background_svg, 
                                                          BinaryData::knob_background_svgSize);
    knobPointerDrawable = Drawable::createFromImageData(BinaryData::knob_pointer_svg, 
                                                       BinaryData::knob_pointer_svgSize);
    
    // Set color scheme
    setColour(Slider::trackColourId, Colors::metalDark);
    setColour(Slider::thumbColourId, Colors::knobBrass);
    setColour(Slider::textBoxTextColourId, Colors::textWhite);
    setColour(Slider::textBoxBackgroundColourId, Colors::background);
    
    setColour(TextButton::buttonColourId, Colors::metalLight);
    setColour(TextButton::textColourOffId, Colors::textWhite);
    setColour(TextButton::textColourOnId, Colors::textGold);
    
    setColour(ComboBox::backgroundColourId, Colors::background);
    setColour(ComboBox::textColourId, Colors::textWhite);
    setColour(ComboBox::outlineColourId, Colors::metalLight);
    
    setColour(Label::textColourId, Colors::textWhite);
    setColour(Label::backgroundColourId, Colours::transparentBlack);
}

ProAutoTuneLookAndFeel::~ProAutoTuneLookAndFeel()
{
}

void ProAutoTuneLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int width, int height,
                                             float sliderPosProportional, float rotaryStartAngle,
                                             float rotaryEndAngle, Slider& slider)
{
    Rectangle<float> bounds(x, y, width, height);
    float diameter = jmin(width, height) * 0.8f;
    Rectangle<float> knobBounds = bounds.withSizeKeepingCentre(diameter, diameter);
    
    // Draw vintage-style knob
    drawVintageKnob(g, knobBounds, sliderPosProportional, rotaryStartAngle, rotaryEndAngle, 
                   Colors::knobBrass);
}

void ProAutoTuneLookAndFeel::drawLinearSlider(Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float minSliderPos, float maxSliderPos,
                                             const Slider::SliderStyle style, Slider& slider)
{
    Rectangle<float> bounds(x, y, width, height);
    
    if (slider.isBar())
    {
        g.setColour(Colors::knobBrass);
        g.fillRect(Rectangle<float>(static_cast<float>(x), sliderPos,
                                   static_cast<float>(width), static_cast<float>(y + height - sliderPos)));
    }
    else
    {
        // Draw track
        auto trackWidth = jmin(6.0f, slider.isHorizontal() ? height * 0.25f : width * 0.25f);
        Point<float> startPoint(slider.isHorizontal() ? x : x + width * 0.5f,
                               slider.isHorizontal() ? y + height * 0.5f : height + y);
        Point<float> endPoint(slider.isHorizontal() ? width + x : startPoint.x,
                             slider.isHorizontal() ? startPoint.y : y);

        Path backgroundTrack;
        backgroundTrack.startNewSubPath(startPoint);
        backgroundTrack.lineTo(endPoint);
        g.setColour(Colors::metalDark);
        g.strokePath(backgroundTrack, {trackWidth, PathStrokeType::curved, PathStrokeType::rounded});

        // Draw thumb
        auto thumbWidth = getSliderThumbRadius(slider);
        Point<float> thumbPoint(slider.isHorizontal() ? sliderPos : ((float) x + (float) width * 0.5f),
                               slider.isHorizontal() ? ((float) y + (float) height * 0.5f) : sliderPos);

        g.setColour(Colors::knobBrass);
        g.fillEllipse(Rectangle<float>(static_cast<float>(thumbWidth * 2)).withCentre(thumbPoint));
        
        g.setColour(Colors::knobBrassLight);
        g.drawEllipse(Rectangle<float>(static_cast<float>(thumbWidth * 2)).withCentre(thumbPoint), 1.0f);
    }
}

void ProAutoTuneLookAndFeel::drawButtonBackground(Graphics& g, Button& button, 
                                                 const Colour& backgroundColour,
                                                 bool shouldDrawButtonAsHighlighted, 
                                                 bool shouldDrawButtonAsDown)
{
    Rectangle<float> bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    
    drawGlossyButton(g, bounds, shouldDrawButtonAsDown, backgroundColour);
    
    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(Colors::textGold.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds, 3.0f, 2.0f);
    }
}

void ProAutoTuneLookAndFeel::drawButtonText(Graphics& g, TextButton& button,
                                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    Font font(getTextButtonFont(button, button.getHeight()));
    g.setFont(font);
    
    g.setColour(button.findColour(shouldDrawButtonAsDown ? TextButton::textColourOnId
                                                        : TextButton::textColourOffId)
                     .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));

    const int yIndent = jmin(4, button.proportionOfHeight(0.3f));
    const int cornerSize = jmin(button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = roundToInt(font.getHeight() * 0.6f);
    const int leftIndent = jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    const int textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
        g.drawFittedText(button.getButtonText(),
                        leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                        Justification::centred, 2);
}

void ProAutoTuneLookAndFeel::drawComboBox(Graphics& g, int width, int height, bool isButtonDown,
                                         int buttonX, int buttonY, int buttonW, int buttonH,
                                         ComboBox& box)
{
    Rectangle<float> bounds(0, 0, width, height);
    
    // Draw background
    g.setColour(Colors::background);
    g.fillRoundedRectangle(bounds, 3.0f);
    
    // Draw metal frame
    drawMetalFrame(g, bounds, 3.0f);
    
    // Draw arrow
    Rectangle<float> arrowZone(width - 20, 0, 20, height);
    Path path;
    path.startNewSubPath(arrowZone.getCentreX() - 3.0f, arrowZone.getCentreY() - 2.0f);
    path.lineTo(arrowZone.getCentreX(), arrowZone.getCentreY() + 2.0f);
    path.lineTo(arrowZone.getCentreX() + 3.0f, arrowZone.getCentreY() - 2.0f);
    
    g.setColour(Colors::textWhite.withAlpha(box.isEnabled() ? 0.9f : 0.2f));
    g.strokePath(path, PathStrokeType(2.0f, PathStrokeType::curved, PathStrokeType::rounded));
}

void ProAutoTuneLookAndFeel::drawLabel(Graphics& g, Label& label)
{
    g.fillAll(label.findColour(Label::backgroundColourId));

    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const Font font(getLabelFont(label));

        g.setColour(label.findColour(Label::textColourId).withMultipliedAlpha(alpha));
        g.setFont(font);

        Rectangle<int> textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                        jmax(1, (int) (textArea.getHeight() / font.getHeight())),
                        label.getMinimumHorizontalScale());

        g.setColour(label.findColour(Label::outlineColourId).withMultipliedAlpha(alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour(label.findColour(Label::outlineColourId));
    }

    g.drawRect(label.getLocalBounds());
}

void ProAutoTuneLookAndFeel::drawVintageKnob(Graphics& g, Rectangle<float> bounds, float sliderPos,
                                            float rotaryStartAngle, float rotaryEndAngle, 
                                            const Colour& knobColour)
{
    auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto centre = bounds.getCentre();
    
    // Draw knob base with gradient
    ColourGradient baseGradient(knobColour.brighter(0.4f), centre.x, bounds.getY(),
                               knobColour.darker(0.3f), centre.x, bounds.getBottom(), false);
    g.setGradientFill(baseGradient);
    g.fillEllipse(bounds.reduced(2.0f));
    
    // Draw inner highlight
    g.setColour(knobColour.brighter(0.8f).withAlpha(0.6f));
    g.fillEllipse(bounds.reduced(radius * 0.2f).removeFromTop(radius * 0.6f));
    
    // Draw outer ring
    g.setColour(Colors::metalDark);
    g.drawEllipse(bounds.reduced(1.0f), 2.0f);
    
    // Draw pointer
    Path pointer;
    auto pointerLength = radius * 0.7f;
    auto pointerThickness = 3.0f;
    
    pointer.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength * 0.8f);
    
    g.setColour(Colors::metalLight);
    pointer.applyTransform(AffineTransform::rotation(toAngle).translated(centre));
    g.fillPath(pointer);
    
    // Draw center dot
    g.setColour(Colors::background);
    g.fillEllipse(bounds.withSizeKeepingCentre(6.0f, 6.0f));
}

void ProAutoTuneLookAndFeel::drawMetalFrame(Graphics& g, Rectangle<float> bounds, float cornerSize)
{
    // Outer shadow
    g.setColour(Colours::black.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds.expanded(1.0f), cornerSize + 1.0f, 1.0f);
    
    // Main frame
    ColourGradient frameGradient(Colors::metalLight, bounds.getX(), bounds.getY(),
                                Colors::metalDark, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(frameGradient);
    g.drawRoundedRectangle(bounds, cornerSize, 2.0f);
    
    // Inner highlight
    g.setColour(Colors::metalLight.brighter(0.3f).withAlpha(0.7f));
    g.drawRoundedRectangle(bounds.reduced(2.0f), cornerSize - 2.0f, 1.0f);
}

void ProAutoTuneLookAndFeel::drawGlossyButton(Graphics& g, Rectangle<float> bounds, bool isPressed, 
                                             const Colour& baseColour)
{
    float cornerSize = 4.0f;
    
    // Button base
    ColourGradient buttonGradient(isPressed ? baseColour.darker(0.2f) : baseColour.brighter(0.1f),
                                 bounds.getX(), bounds.getY(),
                                 isPressed ? baseColour.brighter(0.1f) : baseColour.darker(0.2f),
                                 bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(buttonGradient);
    g.fillRoundedRectangle(bounds, cornerSize);
    
    // Button highlight (glossy effect)
    if (!isPressed)
    {
        ColourGradient highlight(Colours::white.withAlpha(0.4f), bounds.getX(), bounds.getY(),
                                Colours::white.withAlpha(0.0f), bounds.getX(), bounds.getCentreY(), false);
        g.setGradientFill(highlight);
        g.fillRoundedRectangle(bounds.removeFromTop(bounds.getHeight() * 0.5f), cornerSize);
    }
    
    // Button border
    g.setColour(Colors::metalDark);
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
}

void ProAutoTuneLookAndFeel::drawVUMeter(Graphics& g, Rectangle<float> bounds, float level, bool isInput)
{
    // Background
    g.setColour(Colors::background);
    g.fillRoundedRectangle(bounds, 2.0f);
    
    // Frame
    g.setColour(Colors::metalDark);
    g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
    
    // Level bar
    if (level > 0.0f)
    {
        auto levelBounds = bounds.reduced(2.0f);
        float levelHeight = levelBounds.getHeight() * level;
        auto levelRect = levelBounds.removeFromBottom(levelHeight);
        
        // Color based on level
        Colour levelColour;
        if (level < 0.7f)
            levelColour = Colors::ledGreen;
        else if (level < 0.9f)
            levelColour = Colours::orange;
        else
            levelColour = Colors::ledRed;
        
        // Gradient from dark to bright
        ColourGradient levelGradient(levelColour.darker(0.5f), levelRect.getX(), levelRect.getBottom(),
                                    levelColour, levelRect.getX(), levelRect.getY(), false);
        g.setGradientFill(levelGradient);
        g.fillRoundedRectangle(levelRect, 1.0f);
    }
    
    // Scale marks
    g.setColour(Colors::textWhite.withAlpha(0.5f));
    for (int i = 1; i < 4; ++i)
    {
        float y = bounds.getY() + bounds.getHeight() * i / 4.0f;
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX() + 1, bounds.getRight() - 1);
    }
}

void ProAutoTuneLookAndFeel::createGradientFill(ColourGradient& gradient, Rectangle<float> bounds, 
                                               const Colour& color1, const Colour& color2, bool vertical)
{
    if (vertical)
        gradient = ColourGradient(color1, bounds.getX(), bounds.getY(), 
                                 color2, bounds.getX(), bounds.getBottom(), false);
    else
        gradient = ColourGradient(color1, bounds.getX(), bounds.getY(), 
                                 color2, bounds.getRight(), bounds.getY(), false);
}

void ProAutoTuneLookAndFeel::drawInnerShadow(Graphics& g, Rectangle<float> bounds, float cornerSize)
{
    Path shadowPath = createRoundedPath(bounds, cornerSize);
    
    ColourGradient shadow(Colours::black.withAlpha(0.3f), bounds.getX(), bounds.getY(),
                         Colours::transparentBlack, bounds.getX() + 5, bounds.getY() + 5, true);
    g.setGradientFill(shadow);
    g.fillPath(shadowPath);
}

void ProAutoTuneLookAndFeel::drawOuterGlow(Graphics& g, Rectangle<float> bounds, 
                                          const Colour& glowColour, float glowRadius)
{
    for (int i = 1; i <= glowRadius; ++i)
    {
        float alpha = 1.0f - (float)i / glowRadius;
        g.setColour(glowColour.withAlpha(alpha * 0.3f));
        g.drawRoundedRectangle(bounds.expanded(i), 3.0f + i, 1.0f);
    }
}

Path ProAutoTuneLookAndFeel::createRoundedPath(Rectangle<float> bounds, float cornerSize)
{
    Path path;
    path.addRoundedRectangle(bounds, cornerSize);
    return path;
}
