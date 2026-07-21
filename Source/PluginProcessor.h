#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "ParameterIDs.h"
#include "dsp/EnvelopeFollower.h"
#include "dsp/GateEngine.h"

class AutoDuckAudioProcessor : public juce::AudioProcessor,
                               private juce::AsyncUpdater
{
public:
    AutoDuckAudioProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    /** Live gate state for metering and the (future) duck mascot UI. */
    const autoduck::GateEngine& getGateEngine() const noexcept { return gate; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void handleAsyncUpdate() override;
    void updateLookahead();

    autoduck::EnvelopeFollower envelope;
    autoduck::GateEngine gate;

    juce::AudioBuffer<float> delayBuffer;
    int delayBufferLength = 0;
    int delayWritePos = 0;
    int lookaheadSamples = 0;
    std::atomic<int> pendingLatency { 0 };

    std::atomic<float>* thresholdParam = nullptr;
    std::atomic<float>* depthParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* holdParam = nullptr;
    std::atomic<float>* lookaheadParam = nullptr;
    std::atomic<float>* invertParam = nullptr;

    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutoDuckAudioProcessor)
};
