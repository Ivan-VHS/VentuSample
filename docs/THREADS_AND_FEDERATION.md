# VentuSample — project-thread integration ledger

This ledger consolidates the hip-hop sampling, MPC-character, sound-design, desktop-generator and cross-repository requirements discussed across project threads. The implementation remains an offline procedural audio workstation.

## Implemented and source-backed

| Requirement | Current implementation | Evidence |
|---|---|---|
| Hip-hop style matrix | 13 procedural profiles including Boom Bap, East Coast, Harlem Dip-Trap, G-Unit, Modern/Atlanta Trap, UK Drill/Grime, Memphis, Jazz Rap, G-Funk, Boom Trap and Cripple Trill | `src/generator.cpp`, `docs/ARCHITECTURE.md` |
| Parameterized generation | BPM, key/scale, bars, variation count/seed, swing, density, sampler color, grit, saturation, width, filters, snare offset, bus compression, triplet rolls and humanization | `src/generator.hpp`, `src/mainwindow.cpp` |
| MPC-style sampler color | Deterministic 12/14/16-bit-style quantization/held-frame profiles | `src/generator.cpp` |
| Sample-chop import | Bounded PCM16 WAV import, four-slice beat placement and semitone resampling | `src/generator.cpp`, `tests/test_generator.cpp` |
| Sample-pack export | Stereo PCM16 WAV, per-file peak/RMS/crest and versioned `ventusample.pack.v1` manifest | `src/generator.cpp`, `schemas/ventusample.pack.v1.schema.json` |
| Native desktop surface | Qt 6 Widgets, worker-thread rendering, QSettings preferences, light/dark themes and safe export preflight | `src/main.cpp`, `src/mainwindow.cpp`, `CMakeLists.txt` |
| Verified desktop build | Full Qt app builds and tests on Linux Qt 6 and Windows 11/MSVC CI | `.github/workflows/ci.yml` |

## Federation

- VentuSample → Reaper: partial; producer-side WAV + `ventusample.pack.v1` export exists, but Reaper-side pack consumption is not implemented.
- Reaper → VentuSample: planned render-request adapter.
- VentuSample ↔ Pytyliki: planned explicit C ABI use; no native library is loaded today.
- VentuSample ↔ OPlague: not applicable; no direct audio/driver operation.

See [the shared four-project OCTA-8 topology](docs/OCTA_PYRAMID_FEDERATION.md) and [source integration ledger](docs/SOURCE_INTEGRATION.md).

## Deliberate boundaries

- Rendering is offline; there is no system capture, audio-device control, DAW plugin host or realtime playback.
- PCM16 mono/stereo WAV is the supported import format; 24-bit, float, MP3 and FLAC are rejected.
- Semitone resampling changes both pitch and duration; tempo-preserving time-stretch is not implemented.
- MPC names describe artistic sampler-character approximations, not bit-exact hardware emulation.
- Genre profiles are original procedural patterns, not copied artist recordings or proprietary sample libraries.

## Repositories

[Reaper](https://github.com/Ivan-VHS/Reaper) · [Pytyliki](https://github.com/Ivan-VHS/Pytyliki) · [OPlague](https://github.com/Ivan-VHS/OPlague) · [VentuSample](https://github.com/Ivan-VHS/VentuSample)
