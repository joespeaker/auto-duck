#pragma once

#include <array>
#include <juce_core/juce_core.h>

namespace autoduck
{

struct Preset
{
    const char* name;
    float thresholdDb, depthDb, attackMs, releaseMs, holdMs, lookaheadMs;
    bool invert;
};

// Depth of -60 means full mute (true silence) — see GateEngine::fullMuteDepthDb.
inline constexpr std::array<Preset, 4> factoryPresets {{
    { "Init",        -30.0f, -60.0f,   5.0f, 120.0f, 20.0f, 0.0f, false },
    { "Hard Cutoff", -40.0f, -60.0f,   0.1f,  50.0f, 10.0f, 5.0f, false },
    { "Trance Gate", -35.0f, -60.0f,   1.0f,  80.0f,  0.0f, 2.0f, true  },
    { "Subtle Duck", -30.0f,  -9.0f,  20.0f, 250.0f, 50.0f, 0.0f, false },
}};

} // namespace autoduck
