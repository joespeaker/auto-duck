#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace autoduck
{

/** Dark panel look with amber-accented rotary knobs. */
class AutoDuckLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static constexpr juce::uint32 background = 0xff1f2226;
    static constexpr juce::uint32 panel      = 0xff2a2e34;
    static constexpr juce::uint32 outline    = 0xff3a3f46;
    static constexpr juce::uint32 accent     = 0xffffb547;
    static constexpr juce::uint32 textDim    = 0xff9aa2ad;

    AutoDuckLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (background));
        setColour (juce::Slider::textBoxTextColourId, juce::Colour (textDim));
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Label::textColourId, juce::Colour (textDim));
        setColour (juce::TextButton::buttonColourId, juce::Colour (panel));
        setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff43d6e8));
        setColour (juce::TextButton::textColourOffId, juce::Colour (textDim));
        setColour (juce::TextButton::textColourOnId, juce::Colour (0xff10240f).withAlpha (1.0f));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override
    {
        const auto area = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
        const float radius = juce::jmin (area.getWidth(), area.getHeight()) / 2.0f;
        const auto centre = area.getCentre();
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const float arcRadius = radius - 3.0f;
        const juce::PathStrokeType stroke (4.0f, juce::PathStrokeType::curved,
                                           juce::PathStrokeType::rounded);

        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                             rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour (outline));
        g.strokePath (track, stroke);

        juce::Path value;
        value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                             rotaryStartAngle, angle, true);
        g.setColour (juce::Colour (accent));
        g.strokePath (value, stroke);

        const float capRadius = radius * 0.62f;
        g.setGradientFill (juce::ColourGradient::vertical (
            juce::Colour (0xff4a5058), centre.y - capRadius,
            juce::Colour (0xff2e3339), centre.y + capRadius));
        g.fillEllipse (centre.x - capRadius, centre.y - capRadius,
                       capRadius * 2.0f, capRadius * 2.0f);

        const auto pointerEnd = centre.getPointOnCircumference (capRadius - 3.0f, angle);
        const auto pointerStart = centre.getPointOnCircumference (capRadius * 0.35f, angle);
        g.setColour (juce::Colour (accent));
        g.drawLine ({ pointerStart, pointerEnd }, 3.0f);
    }
};

} // namespace autoduck
