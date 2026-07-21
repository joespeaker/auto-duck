#include "PluginEditor.h"

AutoDuckAudioProcessorEditor::AutoDuckAudioProcessorEditor (AutoDuckAudioProcessor& p)
    : AudioProcessorEditor (p), genericEditor (p)
{
    titleLabel.setText ("Auto-Duck", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);
    addAndMakeVisible (genericEditor);

    setSize (420, 400);
}

void AutoDuckAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AutoDuckAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    titleLabel.setBounds (area.removeFromTop (40));
    genericEditor.setBounds (area);
}
