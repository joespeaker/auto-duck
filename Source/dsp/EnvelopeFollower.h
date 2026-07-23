#pragma once

#include <cmath>

namespace autoduck
{

/**
    RMS envelope follower.

    Tracks the running mean-square of the input with a one-pole smoother and
    returns the root, giving a low-jitter level estimate that suits percussive
    sidechain material better than a raw peak detector.
*/
class EnvelopeFollower
{
public:
    void prepare (double newSampleRate, double windowMs = 10.0)
    {
        sampleRate = newSampleRate;
        coeff = 1.0 - std::exp (-1.0 / (0.001 * windowMs * sampleRate));
        reset();
    }

    void reset() noexcept { meanSquare = 0.0; }

    /** Feeds one (mono-summed) sample and returns the current RMS level. */
    float process (float sample) noexcept
    {
        const double sq = static_cast<double> (sample) * sample;
        meanSquare += coeff * (sq - meanSquare);
        return static_cast<float> (std::sqrt (meanSquare));
    }

    float getLevel() const noexcept { return static_cast<float> (std::sqrt (meanSquare)); }

private:
    double sampleRate = 44100.0;
    double coeff = 0.0;
    double meanSquare = 0.0;
};

} // namespace autoduck
