#pragma once

#include <atomic>
#include <cmath>

namespace autoduck
{

/**
    Sidechain gate state machine and gain computer.

    Feed it the sidechain envelope level once per sample; it returns the gain
    to apply to the main signal. Handles threshold triggering, the hold timer
    (anti-chatter), duck/invert mode mapping, exponential attack/release gain
    smoothing, and snapping to true zero at full depth so a hard cutoff is
    genuinely silent.
*/
class GateEngine
{
public:
    struct Parameters
    {
        float thresholdDb = -30.0f;
        float depthDb     = -60.0f;   // -60 on the dial means -inf (gain 0)
        float attackMs    = 5.0f;
        float releaseMs   = 120.0f;
        float holdMs      = 20.0f;
        bool  invert      = false;
    };

    // Dial position at which Depth becomes a true full mute rather than a dB value.
    static constexpr float fullMuteDepthDb = -60.0f;

    void prepare (double newSampleRate)
    {
        sampleRate = newSampleRate;
        reset();
    }

    void reset() noexcept
    {
        gain = 1.0f;
        triggered = false;
        holdCounter = 0;
        publishState();
    }

    void setParameters (const Parameters& p) noexcept
    {
        thresholdLinear = dbToGain (p.thresholdDb);
        depthGain = p.depthDb <= fullMuteDepthDb + 0.01f ? 0.0f : dbToGain (p.depthDb);
        attackCoeff  = timeToCoeff (p.attackMs);
        releaseCoeff = timeToCoeff (p.releaseMs);
        holdSamples  = static_cast<int> (0.001 * p.holdMs * sampleRate);
        invert = p.invert;
    }

    /** Computes the main-signal gain for one sample from the sidechain level. */
    float process (float sidechainLevel) noexcept
    {
        const bool rawTrigger = sidechainLevel > thresholdLinear;

        if (holdCounter > 0)
            --holdCounter;
        else if (rawTrigger != triggered)
        {
            triggered = rawTrigger;
            holdCounter = holdSamples;
        }

        const bool gateOpen = invert ? triggered : ! triggered;
        const float target = gateOpen ? 1.0f : depthGain;

        // Attack governs movement into gain reduction, release the return to unity.
        const float coeff = target < gain ? attackCoeff : releaseCoeff;
        gain += coeff * (target - gain);

        // Exponential ramps never quite land; snap once close enough. Reaching
        // exactly 0 matters for the "full mute is genuinely silent" guarantee.
        if (target == 0.0f && gain < 1.0e-4f)
            gain = 0.0f;
        else if (target == 1.0f && gain > 1.0f - 1.0e-4f)
            gain = 1.0f;

        publishState();
        return gain;
    }

    // Read from any thread (for metering / the future duck mascot UI).
    float currentGain() const noexcept      { return uiGain.load (std::memory_order_relaxed); }
    bool  isTriggered() const noexcept      { return uiTriggered.load (std::memory_order_relaxed); }

private:
    static float dbToGain (float db) noexcept { return std::pow (10.0f, db / 20.0f); }

    float timeToCoeff (float ms) const noexcept
    {
        return 1.0f - std::exp (static_cast<float> (-1.0 / (0.001 * ms * sampleRate)));
    }

    void publishState() noexcept
    {
        uiGain.store (gain, std::memory_order_relaxed);
        uiTriggered.store (triggered, std::memory_order_relaxed);
    }

    double sampleRate = 44100.0;

    float thresholdLinear = 0.0316f;
    float depthGain = 0.0f;
    float attackCoeff = 1.0f;
    float releaseCoeff = 1.0f;
    int holdSamples = 0;
    bool invert = false;

    float gain = 1.0f;
    bool triggered = false;
    int holdCounter = 0;

    std::atomic<float> uiGain { 1.0f };
    std::atomic<bool>  uiTriggered { false };
};

} // namespace autoduck
