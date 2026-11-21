# VentuSample — Native Hip-Hop Sample Foundry

**VentuSample is an offline-first C++20 / Qt 6 workstation for procedural hip-hop sample-pack creation.** It renders stereo audio locally and exports PCM16 WAV files plus a versioned JSON manifest. A fixed seed makes supported variations repeatable.

## Generation capabilities

- Full loops, melody-only loops, drum patterns, bass parts and multi-one-shot kits.
- Boom Bap, Billionaire Golden Era, East Coast, Harlem Dip-Trap, G-Unit, Modern Trap, Atlanta Trap, UK Drill, UK Grime, Memphis, Jazz Rap, G-Funk, Boom Trap and Cripple Trill profiles.
- Musical controls: tempo, bars, key, scale, sample rate and output type.
- Sound controls: sampler character, timbre, swing, density, grit, saturation, stereo width, filters, snare timing and bus-compression ratio.
- Performance controls: triplet rolls, micro-humanization and optional texture.
- Optional PCM16 mono/stereo WAV source import, four-slice chop placement and semitone resampling.
- Deterministic variations, phrase turnarounds, shared drum/melody swing timing, stereo-linked bus compression, WAV export and peak/RMS/crest measurements.
- QMR-20 quality/cohesion/rhythm framework with regression coverage; pack metadata remains compatible with ventusample.pack.v1.

## Desktop workflow

1. Select a sound profile and output type.
2. Set the key, scale, tempo, bar count, seed and variation count.
3. Shape groove, density, timbre and signal processing.
4. Optionally load a supported PCM16 WAV source for chop-based generation.
5. Choose an output directory and confirm pack creation.
6. Review the generated WAV files, manifest, summary and operation log.

Generation runs on a worker thread so the interface remains responsive. The native application is local and does not require network access for rendering.

## Visual design: Duck Sovereign Canon

The Qt Widgets application follows a stable workstation hierarchy: sound identity, musical grid and structure, groove and texture, processing, optional source input, then export and result. The theme uses the approved white/blue/red values, local serif headings with Cyrillic support, local sans-serif controls, clear keyboard focus and 2–4 px geometry. The light/dark switch changes semantic tokens only; both modes remain inside the six-value palette.

See [docs/DUCK_SOVEREIGN_CANON.md](docs/DUCK_SOVEREIGN_CANON.md) for the normative color, contrast, typography, layout, accessibility and offline requirements.

## Build

Requirements: CMake 3.21+, a C++20 compiler and (for the desktop target) Qt 6 Widgets.

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
~~~

When Qt 6 Widgets is unavailable, the portable DSP core and test executable still build; CMake reports that the desktop target was skipped. On Windows, use a Qt 6 desktop development kit and a current MSVC or MinGW toolchain.

## Input and output constraints

- Output: PCM16 WAV plus ventusample_manifest.json.
- Current import: PCM16 mono/stereo WAV up to 128 MiB.
- 24-bit, floating-point WAV, MP3 and FLAC imports are not supported by the current importer.
- Parameter ranges and sample-rate limits are validated before rendering.
- The mastering stage applies a −0.3 dBFS sample-peak ceiling.
- The reference renderer is portable scalar C++20; no implicit native-library loading is performed.
- Genre profiles are original procedural patterns, not copies of artist recordings or proprietary sample libraries.

## Engineering boundaries

Offline rendering does not imply source-sample rights or commercial clearance. A SHA-256 digest identifies bytes; it does not authenticate a publisher. The application does not upload user audio or run background audio capture. Direct VentuSample-to-Pytyliki ABI and Reaper render-request automation remain planned, not implemented. OPlague device/driver diagnostics are not coupled to audio generation.

## References

- [Architecture](docs/ARCHITECTURE.md)
- [Quality, cohesion and rhythm](docs/QUALITY_MONOLITH_RHYTHM.md)
- [Engineering baseline](docs/ENGINEERING_BASELINE.md)
- [User guide](docs/USER_GUIDE_RU.md)
- [Reaper pack audit guide](docs/REAPER_PACK_AUDIT.md)
- [Quality and commercial readiness](docs/QUALITY_COMMERCIAL_STANDARD.md)
- [Four-project OCTA-8 federation](https://github.com/Ivan-VHS/Reaper/blob/main/docs/PROJECT_FEDERATION_MATRIX.md)

## License

See [LICENSE](LICENSE).