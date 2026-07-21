// Dev-only DSP smoke test: runs the real AutoDuckAudioProcessor offline and
// verifies the core gate behavior against the requirements. Exits non-zero on
// failure so it can run in CI.

#include <cmath>
#include <iostream>

#include "../Source/PluginProcessor.h"

namespace
{
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 512;
    constexpr float mainAmp = 0.25f;

    int failures = 0;

    void expect (bool condition, const std::string& name, const std::string& detail)
    {
        std::cout << (condition ? "PASS  " : "FAIL  ") << name
                  << "  (" << detail << ")\n";
        if (! condition)
            ++failures;
    }

    void setParam (AutoDuckAudioProcessor& proc, const char* id, float value)
    {
        auto* param = proc.apvts.getParameter (id);
        param->setValueNotifyingHost (param->convertTo0to1 (value));
    }

    /** Runs blocks of a main sine + sidechain sine at the given amplitude and
        returns the output/input amplitude ratio measured over the last block. */
    float runBlocks (AutoDuckAudioProcessor& proc, float sidechainAmp, int numBlocks)
    {
        juce::AudioBuffer<float> buffer (4, blockSize);
        juce::MidiBuffer midi;
        double lastSumSquares = 0.0;

        for (int b = 0; b < numBlocks; ++b)
        {
            for (int i = 0; i < blockSize; ++i)
            {
                const auto n = static_cast<double> (b * blockSize + i);
                const auto mainSample = static_cast<float> (mainAmp * std::sin (2.0 * juce::MathConstants<double>::pi * 220.0 * n / sampleRate));
                const auto scSample   = static_cast<float> (sidechainAmp * std::sin (2.0 * juce::MathConstants<double>::pi * 440.0 * n / sampleRate));
                buffer.setSample (0, i, mainSample);
                buffer.setSample (1, i, mainSample);
                buffer.setSample (2, i, scSample);
                buffer.setSample (3, i, scSample);
            }

            proc.processBlock (buffer, midi);

            lastSumSquares = 0.0;
            for (int i = 0; i < blockSize; ++i)
                lastSumSquares += buffer.getSample (0, i) * buffer.getSample (0, i);
        }

        const auto outRms = std::sqrt (lastSumSquares / blockSize);
        const auto inRms = mainAmp / juce::MathConstants<double>::sqrt2;
        return static_cast<float> (outRms / inRms);
    }

    std::unique_ptr<AutoDuckAudioProcessor> makeProcessor (float depthDb, bool invert,
                                                           float holdMs = 0.0f, float lookaheadMs = 0.0f)
    {
        auto proc = std::make_unique<AutoDuckAudioProcessor>();
        setParam (*proc, autoduck::ParameterIDs::threshold, -30.0f);
        setParam (*proc, autoduck::ParameterIDs::depth, depthDb);
        setParam (*proc, autoduck::ParameterIDs::attack, 1.0f);
        setParam (*proc, autoduck::ParameterIDs::release, 20.0f);
        setParam (*proc, autoduck::ParameterIDs::hold, holdMs);
        setParam (*proc, autoduck::ParameterIDs::lookahead, lookaheadMs);
        setParam (*proc, autoduck::ParameterIDs::invert, invert ? 1.0f : 0.0f);
        proc->prepareToPlay (sampleRate, blockSize);
        return proc;
    }
}

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    {
        auto proc = makeProcessor (-60.0f, false);
        const auto gain = runBlocks (*proc, 0.0f, 20);
        expect (gain > 0.99f, "duck mode passes when sidechain is silent",
                "gain ratio " + std::to_string (gain));
    }

    {
        auto proc = makeProcessor (-60.0f, false);
        const auto gain = runBlocks (*proc, 0.5f, 40);
        expect (gain == 0.0f, "full depth is genuinely silent (exact zero output)",
                "gain ratio " + std::to_string (gain));
    }

    {
        auto proc = makeProcessor (-12.0f, false);
        const auto gain = runBlocks (*proc, 0.5f, 40);
        const auto expected = std::pow (10.0f, -12.0f / 20.0f);
        expect (std::abs (gain - expected) < 0.03f, "partial depth ducks to -12 dB",
                "gain ratio " + std::to_string (gain) + " expected " + std::to_string (expected));
    }

    {
        auto proc = makeProcessor (-60.0f, true);
        const auto gainLoud = runBlocks (*proc, 0.5f, 40);
        expect (gainLoud > 0.99f, "invert mode passes while sidechain is active",
                "gain ratio " + std::to_string (gainLoud));

        const auto gainQuiet = runBlocks (*proc, 0.0f, 40);
        expect (gainQuiet == 0.0f, "invert mode mutes when sidechain goes silent",
                "gain ratio " + std::to_string (gainQuiet));
    }

    {
        // Hold: a 10 ms burst with 400 ms hold must keep the gate closed well
        // past the burst, then release once the hold expires.
        auto proc = makeProcessor (-60.0f, false, 400.0f);
        runBlocks (*proc, 0.5f, 1);                       // ~10.7 ms trigger burst
        const auto during = runBlocks (*proc, 0.0f, 18);  // ~192 ms into the hold
        expect (during < 0.1f, "hold keeps gate closed after trigger ends",
                "gain ratio " + std::to_string (during));

        const auto after = runBlocks (*proc, 0.0f, 60);   // well past hold + release
        expect (after > 0.95f, "gate reopens after hold expires",
                "gain ratio " + std::to_string (after));
    }

    {
        auto proc = makeProcessor (-60.0f, false, 0.0f, 10.0f);
        const auto expected = static_cast<int> (0.001 * 10.0 * sampleRate);
        expect (proc->getLatencySamples() == expected, "lookahead reports latency to host",
                std::to_string (proc->getLatencySamples()) + " samples, expected " + std::to_string (expected));
    }

    std::cout << (failures == 0 ? "\nAll DSP smoke tests passed.\n"
                                : "\n" + std::to_string (failures) + " test(s) FAILED.\n");
    return failures == 0 ? 0 : 1;
}
