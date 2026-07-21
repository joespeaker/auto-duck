#pragma once

#include <array>
#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"
#include "ui/AutoDuckLookAndFeel.h"
#include "ui/DuckMascot.h"

class AutoDuckAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit AutoDuckAudioProcessorEditor (AutoDuckAudioProcessor&);
    ~AutoDuckAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    AutoDuckAudioProcessor& processor;
    autoduck::AutoDuckLookAndFeel lookAndFeel;

    juce::Label titleLabel;
    juce::Label modeLabel;
    autoduck::DuckMascot mascot;
    std::array<Knob, 6> knobs;
    juce::TextButton invertButton { "INVERT" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> invertAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutoDuckAudioProcessorEditor)
};
