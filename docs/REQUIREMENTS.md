# Auto-Duck — Requirements

**Project type:** Audio Unit (AU) plugin using JUCE (C++), targeting Logic Pro on macOS. Also
builds as VST3 for cross-DAW use via JUCE's multi-format export; AU is the priority target.

**Core concept:** A sidechain-triggered gate that cuts (or, in Invert mode, passes) the main audio
output based on the level of a sidechain input signal.

## Signal Flow

- The plugin has a **main input/output bus** (the track being processed — e.g. bass, pad, FX) and a
  **sidechain input bus** (e.g. vocal or kick drum, routed via the host's sidechain bus system).
- An **envelope follower** tracks the sidechain signal's level in real time.
- When the envelope crosses **Threshold**, the plugin applies gain reduction to the main output
  according to the current mode.

## Modes

- **Duck mode** (default): sidechain active → main output is cut (down to Depth amount, up to full
  mute).
- **Invert mode** (toggle): sidechain active → main output passes through at full volume;
  sidechain inactive → main output is cut.

## Parameters

| Parameter | Range | Notes |
|-----------|-------|-------|
| Threshold | −60 dB to 0 dB | Sidechain level needed to trigger |
| Depth | 0 dB to −∞ (full mute) | Amount of gain reduction applied when triggered |
| Attack | 0.1 ms – 500 ms | Time to reach full gain reduction after trigger |
| Release | 1 ms – 2000 ms | Time to return to unity gain after trigger ends |
| Hold | 0 ms – 500 ms | Minimum time before the plugin can re-trigger (prevents chatter) |
| Lookahead | 0 ms – 20 ms | Optional; adds reported latency to host. Catches the trigger before the transient hits |
| Invert | on/off toggle | Flips Duck mode ↔ Invert mode |

## DSP Behavior

- Envelope follower uses a standard peak or **RMS detector** (RMS preferred for less jitter on
  percussive triggers).
- Gain reduction curve must ramp smoothly per Attack/Release — **no clicks or zipper noise** (use a
  smoothed value / linear or exponential ramp, not instant gain jumps).
- Depth at full mute must be **genuinely silent** (not just −60 dB), so a "hard cutoff" use case
  behaves as expected.
- **Hold** prevents the gate from re-opening/closing faster than the set time even if the sidechain
  signal is choppy.

## UI Requirements

- **Custom UI** (not default JUCE generic sliders) featuring a **robotic duck mascot** as the
  visual centerpiece.
- The duck's pose/state reflects current gain reduction in real time (e.g. head dips lower as gain
  reduction increases).
- Visual distinction between Duck mode and Invert mode (e.g. duck orientation flips, or
  color/lighting changes).
- **Trigger indicator** (visual flash/light) when the sidechain crosses threshold.
- Standard knobs/sliders for Threshold, Depth, Attack, Release, Hold, Lookahead, laid out around or
  below the mascot.

## Build / Project Setup

- JUCE project using **CMake** (preferred over Projucer for easier version control and CI).
- The AudioProcessor declares a sidechain input bus per JUCE's standard sidechain bus template.
- Includes a basic **preset system** with starter presets: "Hard Cutoff", "Trance Gate",
  "Subtle Duck".
- Target macOS **AU + VST3** output; skip AAX/Windows unless it comes up later.

## Out of Scope for v1

- No MIDI-triggered ducking (sidechain audio input only for now).
- No multiband/frequency-dependent ducking — full-band gain reduction only.
- No preset sharing/cloud sync.
