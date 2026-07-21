#include "DuckMascot.h"

namespace autoduck
{

namespace
{
    const juce::Colour metalLight { 0xffaab2bd };
    const juce::Colour metalDark  { 0xff646b76 };
    const juce::Colour metalEdge  { 0xff3a3f46 };
    const juce::Colour beakOrange { 0xffe8873a };
    const juce::Colour accentDuck   { 0xffffb547 };
    const juce::Colour accentInvert { 0xff43d6e8 };
    const juce::Colour lampRed    { 0xffff5347 };
}

juce::Colour DuckMascot::accentColour() const noexcept
{
    return invert ? accentInvert : accentDuck;
}

void DuckMascot::setState (float gainReduction01, bool triggered, bool invertMode)
{
    const float poseTarget = juce::jlimit (0.0f, 1.0f, gainReduction01);
    const float lampTarget = triggered ? 1.0f : 0.0f;

    // Smooth toward targets so the pose reads as motion, with a fast lamp
    // attack so the trigger flash never feels late.
    const float newPose = pose + 0.35f * (poseTarget - pose);
    const float newLamp = lampTarget > lamp ? lampTarget
                                            : lamp + 0.25f * (lampTarget - lamp);

    const bool dirty = std::abs (newPose - pose) > 0.001f
                    || std::abs (newLamp - lamp) > 0.001f
                    || invertMode != invert;
    pose = newPose;
    lamp = newLamp;
    invert = invertMode;

    if (dirty)
        repaint();
}

void DuckMascot::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float scale = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 200.0f;

    // Soft mode-colored glow behind the duck; brighter while triggered.
    {
        const auto glow = accentColour().withAlpha (0.06f + 0.10f * lamp);
        juce::ColourGradient grad (glow, bounds.getCentre(),
                                   glow.withAlpha (0.0f),
                                   bounds.getCentre().translated (0.0f, 95.0f * scale), true);
        g.setGradientFill (grad);
        g.fillEllipse (bounds.withSizeKeepingCentre (190.0f * scale, 190.0f * scale));
    }

    juce::Graphics::ScopedSaveState save (g);

    // Map the 200x200 drawing space into the component; Invert mode mirrors
    // the duck horizontally so it faces the other way.
    auto transform = juce::AffineTransform::translation (-100.0f, -100.0f)
                         .scaled (invert ? -scale : scale, scale)
                         .translated (bounds.getCentreX(), bounds.getCentreY());
    g.addTransform (transform);

    drawDuck (g);
}

void DuckMascot::drawDuck (juce::Graphics& g) const
{
    const auto accent = accentColour();

    // Ground shadow
    g.setColour (juce::Colours::black.withAlpha (0.30f));
    g.fillEllipse (48.0f, 176.0f, 110.0f, 14.0f);

    // Feet: little rounded flippers
    g.setColour (beakOrange.darker (0.2f));
    g.fillRoundedRectangle (74.0f, 168.0f, 26.0f, 12.0f, 5.0f);
    g.fillRoundedRectangle (110.0f, 168.0f, 26.0f, 12.0f, 5.0f);

    // Legs
    g.setColour (metalDark);
    g.fillRoundedRectangle (84.0f, 154.0f, 7.0f, 18.0f, 3.0f);
    g.fillRoundedRectangle (120.0f, 154.0f, 7.0f, 18.0f, 3.0f);

    // Tail: a chunky triangle at the rear
    {
        juce::Path tail;
        tail.addTriangle (146.0f, 132.0f, 172.0f, 108.0f, 156.0f, 148.0f);
        g.setColour (metalDark);
        g.fillPath (tail);
        g.setColour (metalEdge);
        g.strokePath (tail, juce::PathStrokeType (2.0f));
    }

    // Body: metallic ellipse
    {
        const juce::Rectangle<float> body (50.0f, 98.0f, 110.0f, 64.0f);
        g.setGradientFill (juce::ColourGradient::vertical (metalLight, body.getY(),
                                                           metalDark, body.getBottom()));
        g.fillEllipse (body);
        g.setColour (metalEdge);
        g.drawEllipse (body, 2.5f);
    }

    // Wing panel with rivets
    {
        const juce::Rectangle<float> wing (86.0f, 112.0f, 52.0f, 32.0f);
        g.setColour (metalDark.brighter (0.08f));
        g.fillRoundedRectangle (wing, 12.0f);
        g.setColour (metalEdge);
        g.drawRoundedRectangle (wing, 12.0f, 2.0f);
        for (auto pos : { juce::Point<float> (94.0f, 120.0f),
                          juce::Point<float> (130.0f, 120.0f),
                          juce::Point<float> (94.0f, 136.0f),
                          juce::Point<float> (130.0f, 136.0f) })
        {
            g.setColour (metalEdge);
            g.fillEllipse (pos.x - 2.0f, pos.y - 2.0f, 4.0f, 4.0f);
        }
    }

    // Head group: everything from the neck up dips and tilts with the pose.
    {
        juce::Graphics::ScopedSaveState headSave (g);

        const float dip = pose * 30.0f;
        g.addTransform (juce::AffineTransform::rotation (pose * 0.30f, 78.0f, 100.0f)
                            .translated (0.0f, dip));

        // Neck
        g.setColour (metalDark);
        g.fillRoundedRectangle (68.0f, 76.0f, 20.0f, 34.0f, 8.0f);

        // Antenna with trigger lamp
        g.setColour (metalEdge);
        g.drawLine (76.0f, 46.0f, 80.0f, 26.0f, 3.0f);
        const auto lampColour = lampRed.interpolatedWith (juce::Colour (0xff5a2622), 1.0f - lamp);
        if (lamp > 0.05f)
        {
            g.setColour (lampRed.withAlpha (0.35f * lamp));
            g.fillEllipse (80.0f - 11.0f, 24.0f - 11.0f, 22.0f, 22.0f);
        }
        g.setColour (lampColour);
        g.fillEllipse (80.0f - 5.5f, 24.0f - 5.5f, 11.0f, 11.0f);
        g.setColour (metalEdge);
        g.drawEllipse (80.0f - 5.5f, 24.0f - 5.5f, 11.0f, 11.0f, 1.5f);

        // Head
        {
            const juce::Rectangle<float> head (46.0f, 44.0f, 56.0f, 52.0f);
            g.setGradientFill (juce::ColourGradient::vertical (metalLight, head.getY(),
                                                               metalDark, head.getBottom()));
            g.fillEllipse (head);
            g.setColour (metalEdge);
            g.drawEllipse (head, 2.5f);
        }

        // Visor seam
        g.setColour (metalEdge.withAlpha (0.8f));
        g.drawLine (50.0f, 64.0f, 98.0f, 64.0f, 1.5f);

        // Beak: flat robot bill
        {
            const juce::Rectangle<float> bill (16.0f, 66.0f, 34.0f, 13.0f);
            g.setGradientFill (juce::ColourGradient::vertical (beakOrange.brighter (0.15f), bill.getY(),
                                                               beakOrange.darker (0.25f), bill.getBottom()));
            g.fillRoundedRectangle (bill, 6.0f);
            g.setColour (metalEdge);
            g.drawRoundedRectangle (bill, 6.0f, 1.5f);
        }

        // Eye: LED that glows in the mode color while the gate is passing
        // signal, dimming as the duck ducks.
        {
            const float brightness = juce::jlimit (0.15f, 1.0f, 1.0f - pose * 0.85f);
            g.setColour (juce::Colour (0xff14161a));
            g.fillEllipse (54.0f, 56.0f, 16.0f, 16.0f);
            const auto eye = accent.withMultipliedBrightness (brightness);
            if (lamp > 0.05f)
            {
                g.setColour (eye.withAlpha (0.30f * lamp));
                g.fillEllipse (54.0f - 4.0f, 56.0f - 4.0f, 24.0f, 24.0f);
            }
            g.setColour (eye);
            g.fillEllipse (58.0f, 60.0f, 8.0f, 8.0f);
        }
    }

    // Chest badge in the accent color as an extra mode cue
    g.setColour (accent.withAlpha (0.9f));
    g.fillEllipse (66.0f, 118.0f, 9.0f, 9.0f);
    g.setColour (metalEdge);
    g.drawEllipse (66.0f, 118.0f, 9.0f, 9.0f, 1.2f);
}

} // namespace autoduck
