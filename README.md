# Auto-Duck 🦆

A sidechain-triggered gate plugin (Audio Unit + VST3) built with [JUCE](https://juce.com).

Route a trigger signal (vocal, kick, etc.) into Auto-Duck's sidechain input and it cuts — or, in
**Invert mode**, passes — the main audio based on the sidechain level. Classic uses: ducking a bass
under a kick, gating a pad from a rhythmic trigger, hard-cutting FX returns under vocals.

- **Duck mode** (default): sidechain active → main output is cut (down to *Depth*, up to full mute).
- **Invert mode**: sidechain active → main output passes; sidechain quiet → main output is cut.

See [docs/REQUIREMENTS.md](docs/REQUIREMENTS.md) for the full specification and
[docs/PROGRESS.md](docs/PROGRESS.md) for current project status.

## Parameters

| Parameter | Range | Description |
|-----------|-------|-------------|
| Threshold | −60 dB … 0 dB | Sidechain level needed to trigger |
| Depth | 0 dB … −∞ (full mute) | Gain reduction applied when triggered |
| Attack | 0.1 ms … 500 ms | Time to reach full gain reduction |
| Release | 1 ms … 2000 ms | Time to return to unity gain |
| Hold | 0 ms … 500 ms | Minimum time between trigger state changes (anti-chatter) |
| Lookahead | 0 ms … 20 ms | Pre-empts transients; adds reported latency |
| Invert | on/off | Flips Duck mode ↔ Invert mode |

## Building

Requires CMake ≥ 3.22 and a C++17 compiler. JUCE is fetched automatically via CMake FetchContent.

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

- **macOS**: builds AU (`AutoDuck.component`), VST3, and a Standalone app. The AU installs to
  `~/Library/Audio/Plug-Ins/Components/` (copy it there if `COPY_PLUGIN_AFTER_BUILD` is off).
  Validate with `auval -v aufx Adck Jspk`, then scan in Logic Pro.
- **Linux**: builds VST3 + Standalone (used for CI compile checks; AU is macOS-only).

## Testing without a DAW

Four options, from quickest to most complete:

1. **Standalone app** — built automatically:
   `open "build/AutoDuck_artefacts/Release/Standalone/Auto-Duck.app"`.
   Great for seeing the UI and turning knobs; note the standalone only feeds the **main** bus
   from your input device, so the sidechain stays silent and the duck won't trigger.

2. **DSP smoke tests** — automated proof the gate behaves per spec (ducking depth, true silence
   at full mute, invert mode, hold, latency):
   ```sh
   cmake -B build -DCMAKE_BUILD_TYPE=Release -DAUTODUCK_BUILD_TOOLS=ON
   cmake --build build --target DspSmokeTest
   ./build/DspSmokeTest_artefacts/Release/DspSmokeTest
   ```

3. **auval** (macOS) — Apple's AU validation tool, the same check Logic runs before loading a
   plugin. With the `.component` in `~/Library/Audio/Plug-Ins/Components/`:
   ```sh
   auval -v aufx Adck Jspk
   ```

4. **JUCE AudioPluginHost** — a minimal plugin host with free-form routing, ideal for actually
   *hearing* the sidechain behavior without a DAW:
   ```sh
   cmake -B build -DCMAKE_BUILD_TYPE=Release -DAUTODUCK_BUILD_PLUGINHOST=ON
   cmake --build build --target AudioPluginHost
   open "build/AudioPluginHost/AudioPluginHost_artefacts/Release/AudioPluginHost.app"
   ```
   (On Linux, install `ladspa-sdk` first — the host builds with LADSPA support there.)
   In the host: *Options → Edit the List of Available Plug-ins → Scan* for AU/VST3, then add
   Auto-Duck to the graph. Its input pins are main L/R followed by sidechain L/R — connect a
   music source (e.g. an instrument plugin or your interface's channels 1/2) to the main pins
   and your mic to the sidechain pins, then speak: the duck should dip and the music should duck.

## Using in Logic Pro

1. Insert Auto-Duck on the track you want ducked (e.g. bass).
2. In the plugin header, set the **Side Chain** source to the trigger track (e.g. vocal or kick).
3. Lower **Threshold** until the trigger light responds, then set **Depth** and timing to taste.
