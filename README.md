# VentuSample

**VentuSample** is a native C++20 / Qt 6 desktop workstation for procedural hip-hop sample-pack creation. It renders real stereo audio and exports PCM16 WAV files plus a versioned JSON manifest. The generator works offline and uses a fixed seed for repeatable variations.

## What it generates

- Full loops, melody-only loops, drum patterns, bass parts and multi-one-shot kits.
- Boom Bap, East Coast, Harlem Dip-Trap, G-Unit, Modern Trap, Atlanta Trap, UK Drill, UK Grime, Memphis, Jazz Rap, G-Funk, Boom Trap and Cripple Trill profiles.
- Tempo, bars, key, scale, sampler character, timbre, swing, density, grit, saturation, stereo width, filters, snare timing, bus-compression ratio, triplet rolls, humanization and vinyl dust controls.
- Optional PCM16 WAV chop import with four-slice beat placement and semitone resampling.
- Deterministic variations, four-bar phrase turnarounds, shared drum/melody swing timing, stereo-linked bus compression, WAV export and peak/RMS/crest measurement.
- QMR-20 quality/cohesion/rhythm framework with regression coverage; pack metadata remains compatible with `ventusample.pack.v1`.

## Build

Requirements: CMake 3.21+, C++20 compiler and (for the desktop target) Qt 6 Widgets.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

When Qt 6 Widgets is unavailable, the portable DSP core and test executable still build; CMake reports that the desktop target was skipped. On Windows, use a Qt 6 desktop development kit and a current MSVC or MinGW toolchain.

## Run

Launch `VentuSample` from the build output. Choose a sound profile and output type, set the musical and sound-design parameters, select an export folder and press **Create Sample Pack**. Generation runs on a worker thread so the interface remains responsive. Optional source audio accepts PCM16 mono/stereo WAV up to 128 MiB; 24-bit/float/MP3/FLAC imports are not supported yet. The exported folder contains `.wav` files and `ventusample_manifest.json`.

## Engineering boundaries

- Offline, local rendering; no network telemetry or background audio capture.
- No implicit native-library loading; the reference renderer is portable scalar C++20.
- Bounded parameter ranges and explicit sample-rate limits.
- PCM16 WAV output; the mastering stage applies a −0.3 dBFS sample-peak ceiling.
- Genre profiles are original procedural patterns, not copied artist recordings or proprietary sample libraries.
- A generated pack's SHA-256, when calculated externally, identifies bytes; it does not authenticate a publisher.

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md), [`docs/QUALITY_MONOLITH_RHYTHM.md`](docs/QUALITY_MONOLITH_RHYTHM.md), [`docs/ENGINEERING_BASELINE.md`](docs/ENGINEERING_BASELINE.md), and [`docs/USER_GUIDE_RU.md`](docs/USER_GUIDE_RU.md). Publication requirements are defined in [`docs/DOCUMENTATION_CONTROL.md`](docs/DOCUMENTATION_CONTROL.md).
