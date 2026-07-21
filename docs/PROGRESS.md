# Auto-Duck — Progress

Living status document. Update this as work lands.

**Last updated:** 2026-07-21 (groundwork + custom UI first pass)

## Phase overview

| Phase | Status |
|-------|--------|
| 1. Groundwork — repo, docs, CMake/JUCE project, buses, parameters | ✅ Done |
| 2. Core DSP — envelope follower, gate engine, lookahead, presets | ✅ Done (needs listening tests on real hardware) |
| 3. Validation — macOS CI build, `auval`, Logic Pro smoke test | 🔶 CI in place; `auval`/Logic pending (needs a Mac) |
| 4. Custom UI — robot duck mascot, real-time pose, mode visuals, trigger light | ✅ Done (first pass, programmatic artwork) |
| 5. Polish — final artwork, resizing, manual/screenshots, versioning | ⬜ Not started |

## Done

- [x] Repository scaffolding: README, `.gitignore`, docs (requirements + this file).
- [x] CMake project with JUCE pinned via FetchContent; AU + VST3 + Standalone targets
      (AU on macOS only; Linux builds VST3/Standalone for compile checks).
- [x] `AudioProcessor` with main stereo bus + dedicated sidechain input bus
      (`isBusesLayoutSupported` accepts mono/stereo main with matching layout, mono/stereo sidechain).
- [x] All 7 parameters in an `AudioProcessorValueTreeState` with spec ranges and skews:
      Threshold, Depth (−60 dB position = true −∞/gain 0.0), Attack, Release, Hold, Lookahead, Invert.
- [x] RMS envelope follower (`Source/dsp/EnvelopeFollower.h`).
- [x] Gate engine (`Source/dsp/GateEngine.h`): threshold trigger, hold timer (anti-chatter),
      exponential attack/release gain smoothing (no zipper noise), duck/invert mapping,
      true-zero gain snap at full depth, gain-reduction + trigger state exposed atomically for the future UI.
- [x] Lookahead delay line on the main signal; latency reported to the host via
      `setLatencySamples` (updated off the audio thread via `AsyncUpdater`).
- [x] State save/restore (APVTS XML).
- [x] Factory preset system via host programs: Init, Hard Cutoff, Trance Gate, Subtle Duck.
- [x] GitHub Actions CI: macOS build (AU + VST3, artifacts uploaded) and Linux compile check.
- [x] DSP smoke tests (`tools/dsp_smoke_test.cpp`, run in Linux CI): duck/invert gain mapping,
      true-zero output at full depth, −12 dB partial depth accuracy, hold anti-chatter timing,
      and lookahead latency reporting all verified against the real processor offline.
- [x] **Custom UI (Phase 4, first pass)** — `Source/ui/`:
  - [x] Robot duck mascot drawn programmatically (`DuckMascot`), amber/metal styling.
  - [x] Head dips and tilts with gain reduction in real time (30 Hz UI timer reading the
        gate engine's atomics), with smoothed motion.
  - [x] Invert mode flips the duck horizontally and switches the accent color amber → cyan;
        header shows DUCK MODE / INVERT MODE.
  - [x] Trigger indicator: red antenna lamp + eye glow while the sidechain is above threshold.
  - [x] Custom rotary knobs (`AutoDuckLookAndFeel`) for the six continuous parameters with
        value readouts, plus an INVERT toggle button.
  - [x] Headless UI review tool (`tools/render_mascot.cpp`, `-DAUTODUCK_BUILD_TOOLS=ON`)
        rendering mascot poses and the full editor to PNG under xvfb.

## Pending

- [ ] Run `auval -v aufx Adck Jspk` and a Logic Pro smoke test on real macOS hardware.
- [ ] Listening tests: verify attack/release feel, hold anti-chatter behavior on choppy
      sidechain material, and confirm full-mute is truly silent end-to-end.
- [ ] UI polish (Phase 5): resizable editor, hover/focus states on knobs, possible dedicated
      artwork to replace the programmatic mascot, sidechain level meter.
- [ ] Decide on a license for the repository.
- [ ] Company/manufacturer identity: currently `Joe Speaker` / codes `Jspk`/`Adck` — confirm or change
      **before** first release (changing plugin codes later breaks existing DAW sessions).
- [ ] Consider user presets (save/load) on top of the factory programs.

## Out of scope for v1 (per requirements)

- MIDI-triggered ducking, multiband ducking, preset sharing/cloud sync.

## Notes / decisions

- **JUCE via FetchContent** (pinned tag in `CMakeLists.txt`) instead of a git submodule — simpler
  CI and no submodule init step. Bump the tag deliberately.
- **Depth maps −60 dB slider minimum to gain 0.0** (true silence) and displays it as −∞, satisfying
  the "genuinely silent" requirement while keeping a finite slider range.
- **Hold** is implemented as a minimum dwell time in each trigger state (blocks both re-opening and
  re-closing faster than the hold time), matching the anti-chatter intent.
- Development happens in a Linux container where AU cannot be built; the macOS CI job is the
  compile proof for the real target.
