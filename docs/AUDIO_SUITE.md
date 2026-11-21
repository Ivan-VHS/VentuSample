# Ventu Audio Suite — 66 DSP effects

Ventu Audio Suite is the native offline effects rack built into VentuSample. It is a separate subsystem from the 18 workspace/diagnostic plugins documented in PLUGINS.md.

## Desktop workflow

1. Open **Плагины → Ventu Audio Suite — 66 DSP эффектов**.
2. Import a PCM16 mono or stereo WAV. Mono is converted to dual-mono in memory.
3. Select an effect in the library and click **Добавить эффект**. Repeat to build a serial rack; order matters.
4. Select a rack item and edit its Amount, Wet, Rate, Tone and Width controls. Values are stored per effect instance.
5. Click **Обработать и экспортировать** and choose a new WAV destination. Processing takes place on a worker thread. The imported source is never overwritten.

Effects are deterministic for a fixed source and parameter set. Chains are applied left-to-right to a working copy. If an effect fails, the original caller buffer remains unchanged.

## Catalogue

### I. Space and acoustics (1–11)

| # | Stable ID | Display name | Processing model |
|---:|---|---|---|
| 1 | aetherfold | AETHERFOLD | Multi-tap algorithmic room reflections and damped decay |
| 2 | monolith-diffractor | MONOLITH DIFFRACTOR | Comb/all-pass diffusion |
| 3 | sub-tectonic | SUB-TECTONIC | Low-frequency transmission, filtering and harmonic reinforcement |
| 4 | echo-carton | ECHO-CARTON | Short resonant multi-tap echoes |
| 5 | pendulum-hall | PENDULUM HALL | LFO-modulated delay field |
| 6 | void-chamber | VOID-CHAMBER | Dry foreground with a sparse delayed tail |
| 7 | horizon-panner | HORIZON PANNER | Automated stereo azimuth movement |
| 8 | resonance-loom | RESONANCE LOOM | Tuned recursive resonator |
| 9 | cave-breath | CAVE-BREATH | Envelope-modulated reverberant feedback |
| 10 | glass-prism | GLASS-PRISM | Three-band split and stereo distribution |
| 11 | ocean-bathymetry | OCEAN-BATHYMETRY | Low-pass depth damping and submerged reflections |

### II. Granular time (12–22)

| # | Stable ID | Display name | Processing model |
|---:|---|---|---|
| 12 | chrono-grain | CHRONO-GRAIN | Reversed grain windows |
| 13 | fractal-dust | FRACTAL DUST | Decaying multi-scale echo taps |
| 14 | tape-cancer | TAPE-CANCER | Wow/flutter, saturation, bandwidth loss and dropouts |
| 15 | stutter-weaver | STUTTER-WEAVER | Repeated micro-slice blocks |
| 16 | silence-harvester | SILENCE-HARVESTER | Envelope gate feeding a delayed texture |
| 17 | freeze-crystal | FREEZE-CRYSTAL | Grain-based sample-and-hold |
| 18 | drift-slicer | DRIFT-SLICER | Alternating short and moving delay taps |
| 19 | hourglass-delay | HOURGLASS DELAY | LFO-modulated delay time |
| 20 | micro-glitch-foundry | MICRO-GLITCH FOUNDRY | Quantization, sample holds and deterministic glitches |
| 21 | orbit-cloud | ORBIT-CLOUD | Multi-tap delay with modulation and crossfeed |
| 22 | phantom-loop | PHANTOM LOOP | Envelope-controlled delayed loop reveal |

### III. Timbre and synthesis (23–33)

| # | Stable ID | Display name | Processing model |
|---:|---|---|---|
| 23 | morph-chimera | MORPH-CHIMERA | Filter/nonlinear path crossfade |
| 24 | vocal-sculptor | VOCAL-SCULPTOR | Formant-like resonant band emphasis |
| 25 | brass-forge | BRASS-FORGE | Asymmetric saturation and resonance |
| 26 | sub-gravity | SUB-GRAVITY | Low-frequency reinforcement and rectification |
| 27 | harmonic-weaver | HARMONIC-WEAVER | Nonlinear harmonic shaping |
| 28 | noise-alchemist | NOISE-ALCHEMIST | Deterministic noise through a tone-shaped path |
| 29 | chord-architect | CHORD-ARCHITECT | Octave/fifth-oriented harmonic enhancement |
| 30 | texture-loom | TEXTURE-LOOM | Filtered noise, resonance and delay texture |
| 31 | formant-shift-ultra | FORMANT-SHIFT ULTRA | Moving resonant emphasis |
| 32 | resonator-chorus | RESONATOR-CHORUS | Detuned resonant paths |
| 33 | synth-skin | SYNTH-SKIN | Short comb coloration and damping |

### IV. Dynamics and punch (34–44)

| # | Stable ID | Display name | Processing model |
|---:|---|---|---|
| 34 | transient-hammer | TRANSIENT-HAMMER | Difference-envelope transient emphasis |
| 35 | swing-engine | SWING-ENGINE | Periodic rhythmic gain displacement |
| 36 | tape-saturator-pro | TAPE-SATURATOR PRO | Multi-band-inspired nonlinear saturation |
| 37 | glue-bus-titan | GLUE-BUS TITAN | Linked-stereo envelope compression |
| 38 | pump-sidechain-x | PUMP-SIDECHAIN X | Internal envelope ducking |
| 39 | clip-diamond | CLIP-DIAMOND | Soft saturation or hard clipping |
| 40 | dynamics-sculptor | DYNAMICS-SCULPTOR | Envelope-dependent compression/expansion |
| 41 | low-end-lock | LOW-END LOCK | Centered low frequencies and controlled stereo width |
| 42 | crush-analog | CRUSH-ANALOG | Bit-depth reduction and quantization |
| 43 | headroom-guard | HEADROOM-GUARD | Bounded sample-peak guard |
| 44 | velocity-bloom | VELOCITY-BLOOM | Envelope expansion |

### V. Frequency and phase (45–55)

| # | Stable ID | Display name | Processing model |
|---:|---|---|---|
| 45 | spectral-carver | SPECTRAL-CARVER | Tone-focused dynamic band attenuation |
| 46 | phase-aligner | PHASE-ALIGNER | Controllable all-pass-inspired phase correction |
| 47 | resonance-surgeon | RESONANCE-SURGEON | Narrow-band resonance attenuation |
| 48 | air-sculptor | AIR-SCULPTOR | High-frequency shelf and harmonic lift |
| 49 | sub-harmonic-generator | SUB-HARMONIC GENERATOR | Rectification-derived low harmonic component |
| 50 | dynamic-notch-matrix | DYNAMIC-NOTCH MATRIX | Envelope-controlled notch-like band attenuation |
| 51 | eq-morpher | EQ-MORPHER | Continuous low-pass/high-pass curve morph |
| 52 | de-harsh-pro | DE-HARSH PRO | Upper-mid band reduction |
| 53 | mid-side-architect | MID-SIDE ARCHITECT | Mid/side width with low-frequency centering |
| 54 | filter-cascade | FILTER-CASCADE | Multi-stage filtering and resonance |
| 55 | spectrum-freeze | SPECTRUM-FREEZE | Smoothed band sample-and-hold |

### VI. Generative and rhythm (56–66)

| # | Stable ID | Display name | Processing model |
|---:|---|---|---|
| 56 | groove-mutator | GROOVE-MUTATOR | Deterministic rhythmic gating |
| 57 | poly-rhythm-engine | POLY-RHYTHM ENGINE | Independent 3:2, 5:4 and 7:8 modulation cycles |
| 58 | chaos-modulator | CHAOS-MODULATOR | Bounded deterministic nonlinear attractor |
| 59 | reverse-architect | REVERSE-ARCHITECT | Reversed short segments |
| 60 | drone-weaver | DRONE-WEAVER | Filtered feedback sustain |
| 61 | pitch-drifter | PITCH-DRIFTER | Slow variable-delay modulation |
| 62 | beat-deconstructor | BEAT-DECONSTRUCTOR | Transient/body/tail envelope recombination |
| 63 | echo-matrix | ECHO-MATRIX | Multi-tap stereo cross-feedback |
| 64 | lo-fi-time-capsule | LO-FI TIME CAPSULE | Bandwidth reduction, quantization and noise |
| 65 | harmonic-drift | HARMONIC-DRIFT | Slow harmonic emphasis modulation |
| 66 | sonic-catalyst | SONIC-CATALYST | Soft saturation, presence and stereo detail |

## C++ API

The public API is in src/audio_suite.hpp:

- AudioSuiteBuffer stores stereo float samples and a sample rate.
- AudioPluginDescriptor declares a stable ID, name, category, description, UI concept and version.
- AudioPluginParameters provides normalized Amount, Wet, Rate, Tone and Width controls (0.0–1.0).
- audioPluginCatalog() returns the immutable 66-effect catalogue.
- findAudioPlugin() resolves a stable ID or display name.
- processAudioPlugin() processes one effect.
- processAudioChain() renders a serial chain transactionally.
- loadPcm16Wav() and savePcm16Wav() provide bounded PCM16 RIFF/WAVE I/O.

Example:

```cpp
ventu::AudioSuiteBuffer audio;
std::string error;

if (!ventu::loadPcm16Wav("input.wav", audio, error)) {
    // Handle the import error.
}

std::vector<ventu::AudioPluginChainItem> rack{
    {"tape-saturator-pro", {0.72f, 0.65f, 0.30f, 0.55f, 0.50f}},
    {"aetherfold",         {0.58f, 0.32f, 0.42f, 0.70f, 0.72f}},
    {"low-end-lock",       {0.45f, 0.85f, 0.20f, 0.50f, 0.35f}}
};

if (ventu::processAudioChain(audio, rack, error)) {
    ventu::savePcm16Wav("processed.wav", audio, error);
}
```

## Parameter contract

| Control | Range | Purpose |
|---|---:|---|
| Amount | 0–1 | Effect depth / drive |
| Wet | 0–1 | Processed signal blend |
| Rate | 0–1 | Modulation speed / grain or delay scale |
| Tone | 0–1 | Tone, cutoff or resonant emphasis |
| Width | 0–1 | Stereo field control |

The common control surface keeps the host contract consistent. A future descriptor version can add effect-specific typed controls without changing this base API.

## File and safety limits

- Import: RIFF/WAVE, integer PCM16, mono or stereo, 8–384 kHz.
- Maximum import size: 128 MiB.
- Maximum buffer length: 20,000,000 frames.
- Mono import becomes dual-mono; export is stereo PCM16.
- Non-finite samples and parameter values outside 0–1 are rejected.
- Effects are deterministic scalar DSP and run offline on a worker thread.
- Source audio is not overwritten; processing uses a separate working buffer.
- No audio device capture, network access, implicit DLL/SO loading or GPU dependency is used.

## Honest capability boundary

Names and visual concepts describe a design direction, not a claim that every theoretical concept has been physically solved. AETHERFOLD is an algorithmic reflection processor, not a photo/3D-scan acoustic solver. MORPH-CHIMERA is a deterministic filter/saturation crossfade, not a trained neural model. Spectral and phase processors use scalar time-domain approximations rather than a high-resolution STFT engine. There is no real-time callback, VST3/AU binary or third-party plugin loader in this release.

## Build and tests

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The audio-suite tests verify the 66 unique descriptors, execute every DSP processor, validate transactional chain behavior, reject invalid controls and perform a PCM16 WAV round-trip.
