# VentuSample — architecture and audio contract

**Version:** 0.1.0 · **Status:** functional C++20 reference generator; Qt 6 desktop target

## 1. Product boundary

VentuSample is an offline hip-hop sample-pack generator. It owns musical pattern selection, sound synthesis, sample slicing, PCM conversion, pack naming, metadata and the desktop workflow. It does not own the DAW, system audio device, driver stack, recording capture, online sample marketplace or a real-time plugin host.

The application is split into five independently testable layers:

1. **Desktop shell (`src/main.cpp`, `src/mainwindow.*`)** — native Qt 6 Widgets window, parameter controls, folder dialogs, results, diagnostics and persisted preferences.
2. **Control plane (`ventu::Parameters`, `validate`)** — typed settings, bounded ranges, input format validation and deterministic seed selection.
3. **Audio engine (`src/generator.cpp`)** — scalar C++20 render pipeline with per-generation random state and independent style/target profiles.
4. **Asset I/O** — PCM16 WAV import, slicing and pitch resampling; PCM16 WAV export without silent overwrite of an existing pack; versioned `ventusample.pack.v1` manifest containing the full effective settings snapshot and per-file measurements.
5. **Verification** — deterministic tests for parameter validation, repeatability, variation, peak limits, WAV round trips, sample-chop input and pack export.

The UI delegates pack generation to a worker thread. The audio renderer itself is an offline renderer: it does not claim real-time callback guarantees and it does not access audio hardware.

## 2. Musical domain

### Style profiles

| Profile | Pattern / sound direction |
|---|---|
| Boom Bap | Syncopated kick placement, backbeat snare and restrained melodic density |
| East Coast | Firm kick/snare pocket, chopped-key vocabulary and focused low end |
| Harlem Dip-Trap | Dense bounce with brighter hat activity and pronounced loop movement |
| G-Unit | Sparse, weighty groove and tightly controlled drum density |
| Modern Trap | Half-time snare anchors, rapid hat activity and low-end slides |
| Atlanta Trap | Denser trap pattern, rolls and extended low-end movement |
| UK Drill | Irregular kick movement, half-time backbeat and sliding bass |
| UK Grime | More urgent drum placements and clipped melodic stabs |
| Memphis | Dark minor vocabulary, half-time movement and busy hats |
| Jazz Rap | More spacious groove and longer chord decay |
| G-Funk | Longer note envelopes and a more melodic bass register |
| Boom Trap | Boom-bap backbeat with trap-style subdivisions |
| Cripple Trill | Broken-grid hybrid, irregular accents and triplet detail |
| Billionaire Golden Era | Heavy 90s backbeat, late-snare-ready pocket, weight-forward kick/snare voicing, restrained hats and deterministic crate-slice reassembly |

The Billionaire Golden Era profile consolidates the requested golden-era, crate-rebuild, dragged-boom-bap, raw-crunk weight and premium drum-programming direction into one editable profile. It applies a fixed 0-2-1-3 source-slice order when a user supplies a four-slice WAV, gives kick and snare voices a profile-specific level treatment, and keeps hi-hat activity controlled. The user's swing and snare-offset controls remain authoritative; the profile does not silently rewrite them.

The profiles are procedural starting points, not recordings, MIDI transcriptions or exact emulations of any named artist, label, sampler or record.

### Output types

- **Full Loop:** combined drum, melodic, bass and optional imported-chop layers.
- **Melody:** chord stabs and seeded note variations.
- **Drums:** kick, snare, hats, percussion and optional triplet rolls.
- **Bass:** generated bass pattern with optional glide behavior according to profile.
- **One-Shot Kit:** eight individual hits (kick, snare, hat, clap, rim, perc, bass and pluck) per variation.

## 3. Parameter contract

| Parameter | Accepted values / effect |
|---|---|
| BPM | 60–190 |
| Bars | 1–16 |
| Variations | 1–32; each variation has a reproducible derived seed |
| Key | C through B (12 chromatic roots) |
| Scale | Natural Minor, Harmonic Minor, Dorian, Phrygian, Major, Minor Pentatonic |
| Swing | 50–68% |
| Density / grit / saturation / stereo | 0–100% |
| High-pass | 20–300 Hz |
| Low-pass | 2–20 kHz, above high-pass and below 0.49 × sample rate (Nyquist guard) |
| Snare offset | −30 to +30 ms |
| Bus compression ratio | 1:1–12:1 |
| Sample rate | 22050, 44100, 48000 or 96000 Hz |
| Sampler color | Clean, MPC 2500-style 12-bit/held-frame coloration, MPC 3000XL-style 14-bit quantization, MPC 5000-style 16-bit coloration |
| Source WAV | Optional PCM16 mono/stereo, 8–192 kHz, ≤128 MiB and ≤20 million frames |
| Source mix / pitch | 0–100% mix and −12…+12 semitones |

Sampler choices are artistic quantization/drive approximations. They are not bit-exact models of hardware converters, analog circuitry, firmware or proprietary filter behavior.

## 4. Rendering model

The renderer creates stereo float buffers offline. A sixteenth-step grid determines the main pattern; style profiles supply kick locations, snare anchors, hat probability, half-time character, tone decay and bass weight. The seeded generator provides reproducible accents, note selection, pan, humanization and per-variation differences.

The melodic layer uses bounded scale-degree selection and simple chord shapes. Bass generation uses a low-frequency oscillator with optional pitch glide. Drum voices are procedural transient/body/noise models. The sampler stage applies a selected quantization/hold/drive profile. The master stage applies user HP/LP filtering, envelope-following stereo-bus compression, saturation and a final sample-peak ceiling of −0.3 dBFS.

An imported PCM16 WAV is divided into four source regions. Each bar places those regions as beat-synchronous chops; the selected semitone shift is applied by linear resampling. This changes both pitch and duration, so it is not a tempo-preserving time-stretch algorithm. Imported source material is not uploaded, transmitted or rewritten.

## 5. Audio formats and bounds

- Export: RIFF/WAVE, PCM signed 16-bit little-endian, stereo.
- Import: RIFF/WAVE, PCM signed 16-bit little-endian, mono or stereo.
- Other import encodings (24-bit, float WAV, compressed formats) are rejected explicitly rather than guessed.
- Input file size is bounded before parsing; chunk lengths and sample dimensions are checked before allocation.
- Non-finite samples are sanitized before export. Output file size is checked against RIFF's 32-bit chunk limit.
- Peak, RMS and crest measurements are included in the generated pack manifest per output file.

## 6. Desktop behavior

The UI supports light/dark semantic themes, a scrollable parameter workbench, native folder/file pickers, generation status, result listing, log output and a quick-open action for the export directory and double-click opening of exported WAV/manifest files in their system-default applications. Preferences are stored using `QSettings`, outside the source tree. Generation runs on a worker thread; the window does not claim a real-time audio callback or live preview.

## 7. Build and verification

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Qt 6 Widgets is required for the native desktop executable. The portable DSP core and tests can still be built without Qt by configuring `-DVENTU_BUILD_DESKTOP=OFF`.


## 8. QMR-20 — quality, cohesion and rhythm

VentuSample applies the three-axis framework described in [QUALITY_MONOLITH_RHYTHM.md](QUALITY_MONOLITH_RHYTHM.md). “Monolithic” denotes coherent end-to-end behavior; implementation boundaries remain modular.

- **Quality:** the stereo master uses a shared detector/gain across L/R, retains the −0.3 dBFS sample-peak ceiling and records peak/RMS/crest measurements. The ceiling is not represented as true-peak limiting.
- **Cohesion:** a single validated parameter snapshot and seeded render flow govern scheduling, processing, export and manifest evidence.
- **Rhythm:** drum and melodic accents use the same swing mapping. A deterministic four-bar turnaround can add a pickup kick, a density-gated ghost snare and a short hi-hat fill. Micro-humanization remains separate from the metric grid.

The QMR-20 regression test covers repeatability, phrase duration, swing response and the sample-peak ceiling. The pack contract remains `ventusample.pack.v1`.
