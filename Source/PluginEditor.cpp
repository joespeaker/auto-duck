#include "PluginEditor.h"

namespace IDs = autoduck::ParameterIDs;

AutoDuckAudioProcessorEditor::AutoDuckAudioProcessorEditor (AutoDuckAudioProcessor& p)
    : AudioProcessorEditor (p), processor (p)
{
    setLookAndFeel (&lookAndFeel);

    titleLabel.setText ("AUTO-DUCK", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (titleLabel);

    modeLabel.setJustificationType (juce::Justification::centredRight);
    modeLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    addAndMakeVisible (modeLabel);

    addAndMakeVisible (mascot);

    static constexpr std::array<std::pair<const char*, const char*>, 6> knobDefs {{
        { IDs::threshold, "THRESHOLD" },
        { IDs::depth,     "DEPTH" },
        { IDs::attack,    "ATTACK" },
        { IDs::release,   "RELEASE" },
        { IDs::hold,      "HOLD" },
        { IDs::lookahead, "LOOKAHEAD" },
    }};

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& knob = knobs[i];
        knob.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 78, 18);
        addAndMakeVisible (knob.slider);

        knob.label.setText (knobDefs[i].second, juce::dontSendNotification);
        knob.label.setJustificationType (juce::Justification::centred);
        knob.label.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        addAndMakeVisible (knob.label);

        knob.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.apvts, knobDefs[i].first, knob.slider);
    }

    invertButton.setClickingTogglesState (true);
    addAndMakeVisible (invertButton);
    invertAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, IDs::invert, invertButton);

    setSize (600, 430);
    startTimerHz (30);
}

AutoDuckAudioProcessorEditor::~AutoDuckAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void AutoDuckAudioProcessorEditor::timerCallback()
{
    const auto& gate = processor.getGateEngine();
    const bool invert = processor.apvts.getRawParameterValue (IDs::invert)->load() > 0.5f;

    mascot.setState (1.0f - gate.currentGain(), gate.isTriggered(), invert);

    modeLabel.setText (invert ? "INVERT MODE" : "DUCK MODE", juce::dontSendNotification);
    modeLabel.setColour (juce::Label::textColourId,
                         invert ? juce::Colour (0xff43d6e8)
                                : juce::Colour (autoduck::AutoDuckLookAndFeel::accent));
}

void AutoDuckAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (autoduck::AutoDuckLookAndFeel::background));

    auto header = getLocalBounds().removeFromTop (48);
    g.setColour (juce::Colour (autoduck::AutoDuckLookAndFeel::panel));
    g.fillRect (header);
    g.setColour (juce::Colour (autoduck::AutoDuckLookAndFeel::outline));
    g.fillRect (header.removeFromBottom (1));
}

void AutoDuckAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    auto header = area.removeFromTop (48).reduced (14, 8);
    invertButton.setBounds (header.removeFromRight (86).reduced (0, 4));
    header.removeFromRight (10);
    modeLabel.setBounds (header.removeFromRight (110));
    titleLabel.setBounds (header);

    auto knobStrip = area.removeFromBottom (120).reduced (10, 4);
    const int knobWidth = knobStrip.getWidth() / static_cast<int> (knobs.size());
    for (auto& knob : knobs)
    {
        auto cell = knobStrip.removeFromLeft (knobWidth).reduced (4, 0);
        knob.label.setBounds (cell.removeFromTop (16));
        knob.slider.setBounds (cell);
    }

    mascot.setBounds (area.reduced (8));
}
