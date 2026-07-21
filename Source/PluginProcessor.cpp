#include "PluginProcessor.h"

#include "PluginEditor.h"
#include "Presets.h"

namespace IDs = autoduck::ParameterIDs;

AutoDuckAudioProcessor::AutoDuckAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                          .withInput ("Sidechain", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    thresholdParam = apvts.getRawParameterValue (IDs::threshold);
    depthParam     = apvts.getRawParameterValue (IDs::depth);
    attackParam    = apvts.getRawParameterValue (IDs::attack);
    releaseParam   = apvts.getRawParameterValue (IDs::release);
    holdParam      = apvts.getRawParameterValue (IDs::hold);
    lookaheadParam = apvts.getRawParameterValue (IDs::lookahead);
    invertParam    = apvts.getRawParameterValue (IDs::invert);
}

juce::AudioProcessorValueTreeState::ParameterLayout AutoDuckAudioProcessor::createParameterLayout()
{
    using FloatParam = juce::AudioParameterFloat;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto dbText = [] (float value, int) -> juce::String
    {
        return juce::String (value, 1) + " dB";
    };
    auto depthText = [] (float value, int) -> juce::String
    {
        if (value <= autoduck::GateEngine::fullMuteDepthDb + 0.05f)
            return "-inf";
        return juce::String (value, 1) + " dB";
    };
    auto msText = [] (float value, int) -> juce::String
    {
        return juce::String (value, value < 10.0f ? 2 : 0) + " ms";
    };

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { IDs::threshold, 1 }, "Threshold",
        juce::NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -30.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (dbText)));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { IDs::depth, 1 }, "Depth",
        juce::NormalisableRange<float> (autoduck::GateEngine::fullMuteDepthDb, 0.0f, 0.1f),
        autoduck::GateEngine::fullMuteDepthDb,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (depthText)));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { IDs::attack, 1 }, "Attack",
        juce::NormalisableRange<float> (0.1f, 500.0f, 0.01f, 0.35f), 5.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (msText)));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { IDs::release, 1 }, "Release",
        juce::NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.35f), 120.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (msText)));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { IDs::hold, 1 }, "Hold",
        juce::NormalisableRange<float> (0.0f, 500.0f, 0.1f, 0.5f), 20.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (msText)));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { IDs::lookahead, 1 }, "Lookahead",
        juce::NormalisableRange<float> (0.0f, 20.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (msText)));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { IDs::invert, 1 }, "Invert", false));

    return layout;
}

bool AutoDuckAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mono   = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();

    const auto mainIn  = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != mono && mainOut != stereo)
        return false;
    if (mainIn != mainOut)
        return false;

    const auto sidechain = layouts.getChannelSet (true, 1);
    return sidechain.isDisabled() || sidechain == mono || sidechain == stereo;
}

void AutoDuckAudioProcessor::prepareToPlay (double sampleRate, int)
{
    envelope.prepare (sampleRate);
    gate.prepare (sampleRate);

    const int maxLookahead = static_cast<int> (std::ceil (0.020 * sampleRate));
    delayBufferLength = maxLookahead + 1;
    delayBuffer.setSize (getMainBusNumOutputChannels(), delayBufferLength);
    delayBuffer.clear();
    delayWritePos = 0;

    updateLookahead();
    setLatencySamples (lookaheadSamples);
}

void AutoDuckAudioProcessor::releaseResources()
{
    delayBuffer.setSize (0, 0);
}

void AutoDuckAudioProcessor::updateLookahead()
{
    const int newLookahead = juce::jlimit (
        0, delayBufferLength - 1,
        static_cast<int> (0.001 * lookaheadParam->load() * getSampleRate()));

    if (newLookahead != lookaheadSamples)
    {
        lookaheadSamples = newLookahead;
        pendingLatency.store (newLookahead);
        triggerAsyncUpdate();
    }
}

void AutoDuckAudioProcessor::handleAsyncUpdate()
{
    setLatencySamples (pendingLatency.load());
}

void AutoDuckAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    gate.setParameters ({ thresholdParam->load(),
                          depthParam->load(),
                          attackParam->load(),
                          releaseParam->load(),
                          holdParam->load(),
                          invertParam->load() > 0.5f });
    updateLookahead();

    auto mainBus = getBusBuffer (buffer, true, 0);
    auto sidechain = getBusBuffer (buffer, true, 1);

    const int numSamples = buffer.getNumSamples();
    const int mainChannels = mainBus.getNumChannels();
    const int scChannels = sidechain.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        float scSample = 0.0f;
        for (int ch = 0; ch < scChannels; ++ch)
            scSample += sidechain.getSample (ch, i);
        if (scChannels > 1)
            scSample /= static_cast<float> (scChannels);

        const float gain = gate.process (envelope.process (scSample));

        for (int ch = 0; ch < mainChannels; ++ch)
        {
            float sample = mainBus.getSample (ch, i);

            if (lookaheadSamples > 0 && ch < delayBuffer.getNumChannels())
            {
                delayBuffer.setSample (ch, delayWritePos, sample);
                int readPos = delayWritePos - lookaheadSamples;
                if (readPos < 0)
                    readPos += delayBufferLength;
                sample = delayBuffer.getSample (ch, readPos);
            }

            mainBus.setSample (ch, i, sample * gain);
        }

        if (++delayWritePos >= delayBufferLength)
            delayWritePos = 0;
    }
}

juce::AudioProcessorEditor* AutoDuckAudioProcessor::createEditor()
{
    return new AutoDuckAudioProcessorEditor (*this);
}

int AutoDuckAudioProcessor::getNumPrograms()
{
    return static_cast<int> (autoduck::factoryPresets.size());
}

int AutoDuckAudioProcessor::getCurrentProgram()
{
    return currentProgram;
}

const juce::String AutoDuckAudioProcessor::getProgramName (int index)
{
    if (juce::isPositiveAndBelow (index, getNumPrograms()))
        return autoduck::factoryPresets[static_cast<size_t> (index)].name;
    return {};
}

void AutoDuckAudioProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, getNumPrograms()))
        return;

    currentProgram = index;
    const auto& preset = autoduck::factoryPresets[static_cast<size_t> (index)];

    auto apply = [this] (const char* paramID, float value)
    {
        auto* param = apvts.getParameter (paramID);
        param->setValueNotifyingHost (param->convertTo0to1 (value));
    };

    apply (IDs::threshold, preset.thresholdDb);
    apply (IDs::depth, preset.depthDb);
    apply (IDs::attack, preset.attackMs);
    apply (IDs::release, preset.releaseMs);
    apply (IDs::hold, preset.holdMs);
    apply (IDs::lookahead, preset.lookaheadMs);
    apply (IDs::invert, preset.invert ? 1.0f : 0.0f);
}

void AutoDuckAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void AutoDuckAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        auto state = juce::ValueTree::fromXml (*xml);
        if (state.isValid())
        {
            currentProgram = state.getProperty ("program", 0);
            apvts.replaceState (state);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AutoDuckAudioProcessor();
}
