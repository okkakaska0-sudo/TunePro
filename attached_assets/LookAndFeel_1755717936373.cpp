#include "LookAndFeel.h"

// Color definitions
const Colour ProAutoTuneLookAndFeel::Colors::background          = Colour(0xff0D1117);
const Colour ProAutoTuneLookAndFeel::Colors::surface             = Colour(0xff161B22);
const Colour ProAutoTuneLookAndFeel::Colors::surfaceVariant      = Colour(0xff21262D);
const Colour ProAutoTuneLookAndFeel::Colors::primary             = Colour(0xff00D4FF);
const Colour ProAutoTuneLookAndFeel::Colors::primaryVariant      = Colour(0xff0099CC);
const Colour ProAutoTuneLookAndFeel::Colors::secondary           = Colour(0xff7C3AED);
const Colour ProAutoTuneLookAndFeel::Colors::accent              = Colour(0xffFF2D92);
const Colour ProAutoTuneLookAndFeel::Colors::textPrimary         = Colour(0xffffffff);
const Colour ProAutoTuneLookAndFeel::Colors::textSecondary       = Colour(0xffb8b8c8);
const Colour ProAutoTuneLookAndFeel::Colors::textDisabled        = Colour(0xff666666);
const Colour ProAutoTuneLookAndFeel::Colors::border              = Colour(0xff3d3d5c);
const Colour ProAutoTuneLookAndFeel::Colors::borderFocus         = Colour(0xff4CAF50);
const Colour ProAutoTuneLookAndFeel::Colors::highlight           = Colour(0xff4CAF50);
const Colour ProAutoTuneLookAndFeel::Colors::shadow              = Colour(0x80000000);

ProAutoTuneLookAndFeel::ProAutoTuneLookAndFeel()
{
    // Set default colors
    setColour(ResizableWindow::backgroundColourId, Colors::background);
    setColour(DocumentWindow::backgroundColourId, Colors::background);
    
    // Slider colors
    setColour(Slider::backgroundColourId, Colors::surface);
    setColour(Slider::thumbColourId, Colors::primary);
    setColour(Slider::trackColourId, Colors::border);
    setColour(Slider::rotarySliderFillColourId, Colors::primary);
    setColour(Slider::rotarySliderOutlineColourId, Colors::border);
    setColour(Slider::textBoxBackgroundColourId, Colors::surface);
    setColour(Slider::textBoxTextColourId, Colors::textPrimary);
    setColour(Slider::textBoxOutlineColourId, Colors::border);
    
    // ComboBox colors
    setColour(ComboBox::backgroundColourId, Colors::surface);
    setColour(ComboBox::textColourId, Colors::textPrimary);
    setColour(ComboBox::outlineColourId, Colors::border);
    setColour(ComboBox::buttonColourId, Colors::surfaceVariant);
    setColour(ComboBox::arrowColourId, Colors::textSecondary);
    
    // Button colors
    setColour(TextButton::buttonColourId, Colors::surface);
    setColour(TextButton::textColourOffId, Colors::textPrimary);
    setColour(TextButton::textColourOnId, Colors::textPrimary);
    
    // Label colors
    setColour(Label::textColourId, Colors::textPrimary);
    setColour(Label::backgroundColourId, Colours::transparentBlack);
    
    // PopupMenu colors
    setColour(PopupMenu::backgroundColourId, Colors::surface);
    setColour(PopupMenu::textColourId, Colors::textPrimary);
    setColour(PopupMenu::highlightedBackgroundColourId, Colors::primary);
    setColour(PopupMenu::highlightedTextColourId, Colors::textPrimary);
}

ProAutoTuneLookAndFeel::~ProAutoTuneLookAndFeel()
{
}

void ProAutoTuneLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int width, int height,
                                             float sliderPosProportional, float rotaryStartAngle,
                                             float rotaryEndAngle, Slider& slider)
{
    auto bounds = Rectangle<float>(x, y, width, height).reduced(10.0f);
    auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = jmin(8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;
    auto centre = bounds.getCentre();
    
    // Draw outer ring (track)
    Path backgroundArc;
    backgroundArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                               rotaryStartAngle, rotaryEndAngle, true);
    
    g.setColour(Colors::border);
    g.strokePath(backgroundArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
    
    // Draw filled arc (value)
    if (sliderPosProportional > 0.0f)
    {
        Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                              rotaryStartAngle, toAngle, true);
        
        // Create gradient for the value arc
        ColourGradient gradient(Colors::primary, centre.x - radius, centre.y,
                                     Colors::accent, centre.x + radius, centre.y, false);
        g.setGradientFill(gradient);
        g.strokePath(valueArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }
    
    // Draw inner circle
    auto innerRadius = radius * 0.6f;
    Rectangle<float> innerCircle(centre.x - innerRadius, centre.y - innerRadius,
                                      innerRadius * 2.0f, innerRadius * 2.0f);
    
    // Inner circle gradient
    ColourGradient innerGradient(Colors::surfaceVariant.brighter(0.2f), centre.x, centre.y - innerRadius,
                                      Colors::surface.darker(0.3f), centre.x, centre.y + innerRadius, false);
    g.setGradientFill(innerGradient);
    g.fillEllipse(innerCircle);
    
    // Inner circle border
    g.setColour(Colors::border);
    g.drawEllipse(innerCircle, 2.0f);
    
    // Draw pointer
    Path pointer;
    auto pointerLength = radius * 0.4f;
    auto pointerThickness = 3.0f;
    
    pointer.addRectangle(-pointerThickness * 0.5f, -pointerLength + 5.0f, pointerThickness, pointerLength);
    pointer.applyTransform(AffineTransform::rotation(toAngle).translated(centre.x, centre.y));
    
    g.setColour(Colors::textPrimary);
    g.fillPath(pointer);
    
    // Draw center dot
    auto dotRadius = 4.0f;
    g.setColour(Colors::primary);
    g.fillEllipse(centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    
    // Add glow effect when focused
    if (slider.hasKeyboardFocus(false))
    {
        drawGlowEffect(g, bounds, Colors::primary, 0.8f);
    }
}

void ProAutoTuneLookAndFeel::drawLinearSlider(Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float minSliderPos, float maxSliderPos,
                                             const Slider::SliderStyle style, Slider& slider)
{
    auto trackWidth = jmin(6.0f, (float)jmin(width, height) * 0.25f);
    Point<float> startPoint, endPoint;
    
    if (style == Slider::LinearHorizontal)
    {
        startPoint = {(float)x, (float)y + (float)height * 0.5f};
        endPoint = {(float)(x + width), startPoint.y};
    }
    else
    {
        startPoint = {(float)x + (float)width * 0.5f, (float)(y + height)};
        endPoint = {startPoint.x, (float)y};
    }
    
    Path track;
    track.startNewSubPath(startPoint);
    track.lineTo(endPoint);
    g.setColour(Colors::border);
    g.strokePath(track, {trackWidth, PathStrokeType::curved, PathStrokeType::rounded});
    
    Path filledTrack;
    filledTrack.startNewSubPath(startPoint);
    filledTrack.lineTo({sliderPos, startPoint.y});
    g.setColour(Colors::primary);
    g.strokePath(filledTrack, {trackWidth, PathStrokeType::curved, PathStrokeType::rounded});
    
    // Draw thumb
    auto thumbWidth = getSliderThumbRadius(slider);
    g.setColour(Colors::primary);
    g.fillEllipse(Rectangle<float>(thumbWidth, thumbWidth).withCentre({sliderPos, startPoint.y}));
    
    g.setColour(Colors::textPrimary);
    g.drawEllipse(Rectangle<float>(thumbWidth, thumbWidth).withCentre({sliderPos, startPoint.y}), 1.0f);
}

Label* ProAutoTuneLookAndFeel::createSliderTextBox(Slider& slider)
{
    auto label = LookAndFeel_V4::createSliderTextBox(slider);
    
    label->setFont(getControlFont());
    label->setJustificationType(Justification::centred);
    label->setColour(Label::textColourId, Colors::textPrimary);
    label->setColour(Label::backgroundColourId, Colors::surface);
    label->setColour(Label::outlineColourId, Colors::border);
    
    return label;
}

void ProAutoTuneLookAndFeel::drawComboBox(Graphics& g, int width, int height, bool isButtonDown,
                                         int buttonX, int buttonY, int buttonW, int buttonH,
                                         ComboBox& comboBox)
{
    auto cornerSize = 6.0f;
    auto bounds = Rectangle<int>(0, 0, width, height).toFloat();
    
    // Background gradient
    ColourGradient gradient(Colors::surface.brighter(0.1f), 0.0f, 0.0f,
                                 Colors::surface.darker(0.1f), 0.0f, (float)height, false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, cornerSize);
    
    // Border
    auto borderColor = comboBox.hasKeyboardFocus(false) ? Colors::borderFocus : Colors::border;
    g.setColour(borderColor);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    
    // Button area
    auto buttonArea = Rectangle<float>(buttonX, buttonY, buttonW, buttonH).reduced(2.0f);
    
    if (isButtonDown)
    {
        g.setColour(Colors::surfaceVariant.darker(0.2f));
        g.fillRoundedRectangle(buttonArea, cornerSize * 0.5f);
    }
    
    // Draw arrow
    auto arrowArea = buttonArea.reduced(buttonArea.getWidth() * 0.3f, buttonArea.getHeight() * 0.3f);
    Path arrow;
    arrow.addTriangle(arrowArea.getX(), arrowArea.getY(),
                     arrowArea.getRight(), arrowArea.getY(),
                     arrowArea.getCentreX(), arrowArea.getBottom());
    
    g.setColour(Colors::textSecondary);
    g.fillPath(arrow);
    
    // Add subtle inner shadow
    g.setColour(Colors::shadow.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), cornerSize - 1.0f, 1.0f);
}

void ProAutoTuneLookAndFeel::positionComboBoxText(ComboBox& comboBox, Label& labelToPosition)
{
    labelToPosition.setBounds(8, 1, comboBox.getWidth() - 30, comboBox.getHeight() - 2);
    labelToPosition.setFont(getControlFont());
    labelToPosition.setJustificationType(Justification::centredLeft);
}

void ProAutoTuneLookAndFeel::drawButtonBackground(Graphics& g, Button& button,
                                                 const Colour& backgroundColour,
                                                 bool shouldDrawButtonAsHighlighted,
                                                 bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    auto cornerSize = 6.0f;
    
    Colour baseColor = Colors::surface;
    
    if (shouldDrawButtonAsDown)
    {
        baseColor = Colors::surfaceVariant.darker(0.2f);
    }
    else if (shouldDrawButtonAsHighlighted)
    {
        baseColor = Colors::surfaceVariant.brighter(0.1f);
    }
    
    // Background gradient
    ColourGradient gradient(baseColor.brighter(0.1f), bounds.getCentreX(), bounds.getY(),
                                 baseColor.darker(0.1f), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, cornerSize);
    
    // Border
    Colour borderColor = Colors::border;
    if (button.hasKeyboardFocus(false))
        borderColor = Colors::borderFocus;
    
    g.setColour(borderColor);
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    
    // Inner highlight
    if (shouldDrawButtonAsHighlighted && !shouldDrawButtonAsDown)
    {
        g.setColour(Colors::highlight.withAlpha(0.1f));
        g.fillRoundedRectangle(bounds.reduced(1.0f), cornerSize - 1.0f);
    }
}

void ProAutoTuneLookAndFeel::drawButtonText(Graphics& g, TextButton& button,
                                           bool shouldDrawButtonAsHighlighted,
                                           bool shouldDrawButtonAsDown)
{
    auto font = getControlFont();
    g.setFont(font);
    
    Colour textColor = Colors::textPrimary;
    if (!button.isEnabled())
        textColor = Colors::textDisabled;
    
    g.setColour(textColor);
    
    auto yIndent = jmin(4, button.proportionOfHeight(0.3f));
    auto cornerSize = jmin(button.getHeight(), button.getWidth()) / 2;
    auto fontHeight = roundToInt(font.getHeight() * 0.6f);
    auto leftIndent = jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    auto rightIndent = jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    auto textWidth = button.getWidth() - leftIndent - rightIndent;
    
    if (textWidth > 0)
    {
        g.drawFittedText(button.getButtonText(),
                        leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                        Justification::centred, 2);
    }
}

void ProAutoTuneLookAndFeel::drawLabel(Graphics& g, Label& label)
{
    g.fillAll(label.findColour(Label::backgroundColourId));
    
    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const auto font = getLabelFont(label);
        
        g.setColour(label.findColour(Label::textColourId).withMultipliedAlpha(alpha));
        g.setFont(font);
        
        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
        
        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                        jmax(1, (int)((float)textArea.getHeight() / font.getHeight())),
                        label.getMinimumHorizontalScale());
        
        g.setColour(label.findColour(Label::outlineColourId).withMultipliedAlpha(alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour(label.findColour(Label::outlineColourId));
    }
    
    g.drawRect(label.getLocalBounds());
}

void ProAutoTuneLookAndFeel::drawPopupMenuBackground(Graphics& g, int width, int height)
{
    auto background = findColour(PopupMenu::backgroundColourId);
    g.fillAll(background);
    
    // Add subtle border
    g.setColour(Colors::border);
    g.drawRect(0, 0, width, height, 1);
    
    // Add drop shadow effect
    Path shadowPath;
    shadowPath.addRoundedRectangle(2, 2, width - 4, height - 4, 4.0f);
    DropShadow shadow(Colors::shadow, 8, Point<int>(0, 2));
    shadow.drawForPath(g, shadowPath);
}

void ProAutoTuneLookAndFeel::drawPopupMenuItem(Graphics& g, const Rectangle<int>& area,
                                              bool isSeparator, bool isActive, bool isHighlighted,
                                              bool isTicked, bool hasSubMenu, const String& text,
                                              const String& shortcutKeyText,
                                              const Drawable* icon, const Colour* textColour)
{
    if (isSeparator)
    {
        auto r = area.reduced(5, 0);
        r.removeFromTop(roundToInt(((float)r.getHeight() * 0.5f) - 0.5f));
        
        g.setColour(Colors::border);
        g.fillRect(r.removeFromTop(1));
    }
    else
    {
        auto textColor = (textColour == nullptr ? findColour(PopupMenu::textColourId) : *textColour);
        
        auto maxFontHeight = (float)area.getHeight() / 1.3f;
        
        if (isHighlighted && isActive)
        {
            g.setColour(Colors::primary.withAlpha(0.8f));
            g.fillRect(area.reduced(1));
            
            g.setColour(Colors::textPrimary);
        }
        else
        {
            g.setColour(textColor.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
        }
        
        g.setFont(getControlFont().withHeight(maxFontHeight));
        
        auto iconArea = area.reduced(5).removeFromLeft(roundToInt(maxFontHeight)).toFloat();
        
        if (icon != nullptr)
        {
            icon->drawWithin(g, iconArea, RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
        }
        else if (isTicked)
        {
            // Draw checkmark
            Path tick;
            tick.startNewSubPath(iconArea.getX() + iconArea.getWidth() * 0.2f, iconArea.getCentreY());
            tick.lineTo(iconArea.getCentreX(), iconArea.getY() + iconArea.getHeight() * 0.8f);
            tick.lineTo(iconArea.getRight() - iconArea.getWidth() * 0.2f, iconArea.getY() + iconArea.getHeight() * 0.3f);
            
            g.strokePath(tick, PathStrokeType(2.0f));
        }
        
        if (hasSubMenu)
        {
            auto arrowH = 0.6f * getPopupMenuFont().getHeight();
            auto x = (float)area.getRight() - arrowH * 1.0f;
            auto halfH = (float)area.getHeight() * 0.5f;
            
            Path path;
            path.addTriangle(x, halfH - arrowH * 0.5f,
                           x, halfH + arrowH * 0.5f,
                           x + arrowH * 0.6f, halfH);
            
            g.fillPath(path);
        }
        
        g.drawFittedText(text,
                        area.reduced(iconArea.getWidth() + 5, 0),
                        Justification::centredLeft, 1);
        
        if (shortcutKeyText.isNotEmpty())
        {
            auto f2 = getControlFont().withHeight(maxFontHeight * 0.75f);
            g.setFont(f2);
            g.drawText(shortcutKeyText, area, Justification::centredRight, true);
        }
    }
}

void ProAutoTuneLookAndFeel::drawGlowEffect(Graphics& g, const Rectangle<float>& bounds,
                                           Colour glowColor, float intensity)
{
    const int glowRadius = 10;
    const float alpha = 0.3f * intensity;
    
    for (int i = 0; i < glowRadius; ++i)
    {
        float currentAlpha = alpha * (1.0f - static_cast<float>(i) / glowRadius);
        g.setColour(glowColor.withAlpha(currentAlpha));
        g.drawRoundedRectangle(bounds.expanded(i), 8.0f + i, 1.0f);
    }
}

void ProAutoTuneLookAndFeel::drawGradientBackground(Graphics& g, const Rectangle<int>& bounds,
                                                   Colour topColor, Colour bottomColor)
{
    ColourGradient gradient(topColor, 0.0f, (float)bounds.getY(),
                                 bottomColor, 0.0f, (float)bounds.getBottom(), false);
    g.setGradientFill(gradient);
    g.fillRect(bounds);
}

void ProAutoTuneLookAndFeel::drawRoundedPath(Graphics& g, const Path& path,
                                            Colour fillColor, Colour strokeColor,
                                            float strokeWidth)
{
    g.setColour(fillColor);
    g.fillPath(path);
    
    if (strokeWidth > 0.0f)
    {
        g.setColour(strokeColor);
        g.strokePath(path, PathStrokeType(strokeWidth));
    }
}

Font ProAutoTuneLookAndFeel::getTitleFont() const
{
    return Font("Arial", 24.0f, Font::bold);
}

Font ProAutoTuneLookAndFeel::getControlFont() const
{
    return Font("Arial", 14.0f, Font::plain);
}

Font ProAutoTuneLookAndFeel::getSmallFont() const
{
    return Font("Arial", 12.0f, Font::plain);
}
