#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "PluginProcessor.h"

/**
    Placeholder editor: wraps JUCE's generic parameter panel.

    Phase 4 replaces this with the custom robot-duck UI (mascot pose driven by
    AutoDuckAudioProcessor::getGateEngine(), mode visuals, trigger light).
*/
class AutoDuckAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AutoDuckAudioProcessorEditor (AutoDuckAudioProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::GenericAudioProcessorEditor genericEditor;
    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutoDuckAudioProcessorEditor)
};
