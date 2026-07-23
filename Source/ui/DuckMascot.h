#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace autoduck
{

/**
    The robot duck mascot.

    Drawn programmatically in a normalized 200x200 space and scaled to fit.
    - Head dips as gain reduction increases (0 = upright, 1 = fully ducked).
    - The antenna lamp and eye light up while the sidechain trigger is active.
    - Invert mode flips the duck horizontally and switches the accent color
      from amber to cyan.
*/
class DuckMascot : public juce::Component
{
public:
    /** Called from the editor's UI timer. Values are smoothed internally. */
    void setState (float gainReduction01, bool triggered, bool invertMode);

    void paint (juce::Graphics&) override;

private:
    void drawDuck (juce::Graphics&) const;

    juce::Colour accentColour() const noexcept;

    float pose = 0.0f;   // smoothed head dip, 0..1
    float lamp = 0.0f;   // smoothed trigger lamp intensity, 0..1
    bool invert = false;
};

} // namespace autoduck
