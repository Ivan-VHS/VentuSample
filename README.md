# VentuSample

**VentuSample** is a native C++20 / Qt 6 desktop workstation for procedural hip-hop sample-pack creation. It renders real stereo audio and exports PCM16 WAV files plus a versioned JSON manifest. The generator works offline and uses a fixed seed for repeatable variations.

## What it generates

- Full loops, melody-only loops, drum patterns, bass parts and multi-one-shot kits.
- Boom Bap, Billionaire Golden Era, East Coast, Harlem Dip-Trap, G-Unit, Modern Trap, Atlanta Trap, UK Drill, UK Grime, Memphis, Jazz Rap, G-Funk, Boom Trap and Cripple Trill profiles.
- Tempo, bars, key, scale, sampler character, timbre, swing, density, grit, saturation, stereo width, filters, snare timing, bus-compression ratio, triplet rolls, humanization and vinyl dust controls.
- Optional PCM16 WAV chop import with four-slice beat placement and semitone resampling.
- Billionaire Golden Era crate reconstruction with a deterministic four-slice reassembly order, weighted kick/snare voicing, restrained hats and a late-backbeat-ready swing pocket.
- Deterministic variations, four-bar phrase turnarounds, shared drum/melody swing timing, stereo-linked bus compression, WAV export and peak/RMS/crest measurement.
- QMR-20 quality/cohesion/rhythm framework with regression coverage; pack metadata remains compatible with `ventusample.pack.v1`.
- Reaper interoperability: Reaper validates `ventusample.pack.v1` manifests and compares normalized musical settings and stored peak/RMS/crest measurements through `ventusample_metrics()` and `ventusample_compare()`; the audit does not read WAV assets.

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

## Quality and commercial readiness

VentuSample now includes a 20 × 3 evidence-linked matrix for audio/product quality, operational repeatability and commercial pack release. Run **python tools/quality_commercial.py** or add **--format json**. The report does not certify source-sample rights, market demand, sales or profitability. See [the quality-commercial standard](docs/QUALITY_COMMERCIAL_STANDARD.md).


The readiness report also prints a prioritized commercial action plan. Open items stay explicit: license review, support ownership, release delivery, user validation, cost measurement and any future pricing decision. No revenue, profit or market-fit claim is inferred.

## Federation

See the [four-project OCTA-8 matrix](https://github.com/Ivan-VHS/Reaper/blob/main/docs/PROJECT_FEDERATION_MATRIX.md) and [Reaper pack audit guide](docs/REAPER_PACK_AUDIT.md). Direct VentuSample-to-Pytyliki ABI and Reaper render-request automation remain planned; OPlague device/driver diagnostics are not coupled to offline audio generation.

## Visual design: Duck Sovereign Canon

The Qt Widgets application follows a stable workstation hierarchy: sound identity, musical grid and structure, groove and texture, processing, optional source input, then export and result. The theme uses the approved white/blue/red values, local serif headings with Cyrillic support, local sans-serif controls, clear keyboard focus and 2–4 px geometry. The light/dark switch changes semantic tokens only; both modes remain inside the six-value palette.

See [docs/DUCK_SOVEREIGN_CANON.md](docs/DUCK_SOVEREIGN_CANON.md) for the normative color, contrast, typography, layout, accessibility and offline requirements.

## Anatidae design layer

The shared [Anatidae naming canon](federation/ANATIDAE_NAMING_CANON.md) governs friendly UI vocabulary across the four-project family. The [VentuSample desktop design contract](docs/ANATIDAE_DESKTOP_DESIGN_SYSTEM.md) defines its use without changing audio profiles, parameters or export behavior.

## Market development and positioning

The [market-development standard](docs/MARKET_DEVELOPMENT_STANDARD.md) and [versioned profile](market-development.v1.json) define audience segments, evidence-backed positioning, promotion channels, commercialization gates and privacy-respecting measurement. They do not imply market demand, sales or profitability.

## Improvement and conflict resolution

The [20-control improvement, dissonance, and conflict standard](docs/IMPROVEMENT_CONFLICT_STANDARD.md) turns inconsistencies into evidence-led, testable changes while preserving VentuSample's offline deterministic-rendering, bounded-audio-input, source-rights, and versioned-pack boundaries. Run `python tools/improvement_conflict.py` to validate the machine-readable register. This validates the governance contract; it is not a source-code scanner and does not claim unassessed controls are resolved.
## Correction and defect coverage

The **20 × 3 correction / coverage / defect matrix** preserves all supplied terms and maps them to project-specific targets, acceptance criteria and repository-local evidence. Run `python tools/quality_remediation.py` or add `--format json`; the focused regression suite runs in CI. Evidence-path presence is a traceability check, not proof that behavior is correct or that all defects are gone. See [the remediation standard](docs/QUALITY_REMEDIATION_STANDARD.md).

The repeated term «Исчерпывающесть» is intentionally retained in rows 5 and 18 as supplied; the matrix preserves the original vocabulary rather than silently normalizing it.

## Tonal sample export

Generated tonal loops, melodies and bass files include the selected root and mode in their WAV names (for example, `A_Minor` or `C_Major`). The `ventusample.pack.v2` manifest records the readable `musical_key`, `mode_family` and exact `scale` in the parameter snapshot and on each exported file. Drum-only files remain unlabelled; pitched bass/pluck one-shots carry only their note. Imported source WAVs are not automatically key-detected. The v1 pack schema remains available for existing packs.
