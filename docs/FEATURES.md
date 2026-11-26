# Feature matrix

| Feature | Contract |
|---|---|
| Generator | loops, melody, drums, bass, 8-hit kits |
| Profiles | 14 procedural styles incl. Billionaire Golden Era |
| Golden Era | fixed four-slice order 0–2–1–3, kick/snare voicing, restrained hats |
| Source | PCM16 WAV, mono/stereo, 8–192 kHz, ≤128 MiB, ≤20M frames |
| Chop | 4 slices, semitone resampling ±12; speed changes with pitch |
| Variation | fixed/derived seeds, phrase turns every 4 bars |
| Timing | shared drum/melody swing, snare offset, micro-humanization |
| Processing | filters, saturation, linked stereo bus compression |
| Export | PCM16 WAV + manifest v2; v1 compatibility retained |
| Analysis | peak, RMS, crest per file; offline FFT peak, centroid and 85% rolloff | bounded in-memory analysis; no audio-device access |
| Key labels | key/mode on pitched material; no fabricated drum key |
| Reaper | v1 manifest metrics/comparison; JSON only, no waveform audit |
| QMR-20 | 20 quality × cohesion × rhythm controls and regression gates |

Profiles are original procedural patterns, not artist or recording emulations.

## CRATE Hip-Hop Production Suite

- Crate, Chop, Pocket, Drum, Loop, Arrangement, Texture, Beat Routing, Mix and Print rooms mapped to the existing offline audio core.
- Four starter presets using the current `ventusample.preset.v1` contract: East Coast 93, Boom Bap 88, G-Unit 93 and Golden Era 90.
- Native preset tests load all four JSON examples through the application preset parser.
- Production checks cover source bounds, chop placement, groove, drum balance, loop roles, texture, mix, WAV print and replay.
- This is a hip-hop workflow layer, not a real-time DAW, live recorder or VST/AU host.

Starter files: `examples/hip-hop-presets/`. Full room map and boundaries: [CRATE Production Suite](CRATE_PRODUCTION_SUITE.md) · [Russian edition](CRATE_PRODUCTION_SUITE_RU.md).

## CRATE Hip-Hop Production Suite

- Crate, Chop, Pocket, Drum, Loop, Arrangement, Texture, Beat Routing, Mix and Print rooms mapped to the existing offline audio core.
- Four starter presets using the current ventusample.preset.v1 contract: East Coast 93, Boom Bap 88, G-Unit 93 and Golden Era 90.
- Native preset tests load all four JSON examples through the application preset parser.
- Production checks cover source bounds, chop placement, groove, drum balance, loop roles, texture, mix, WAV print and replay.
- This is a hip-hop workflow layer, not a real-time DAW, live recorder or VST/AU host.

Starter files: examples/hip-hop-presets/. Full room map and boundaries: [CRATE Production Suite](CRATE_PRODUCTION_SUITE.md) · [Russian edition](CRATE_PRODUCTION_SUITE_RU.md).

## Stochastic Router

- 8 weighted DSP effects, each independently selectable in the pool.
- Serial, parallel and hybrid graphs; file, bar and generator-onset triggers.
- Deterministic routing by seed and variation; manifest v2 route summaries.
- Wet ≤65%, feedback ≤72%, 5–20 ms fades, bounded RMS compensation and −0.3 dBFS sample-peak protection.
- Integrated into offline WAV export and exposed as the 19th built-in plugin.

## Quantum Audio Engine

- 133 named audio DSP modules across five scientific/conceptual families.
- 21 reusable C++ DSP kernels with individual module IDs and parameter defaults.
- Ordered offline WAV processing chain with per-module Amount, Tone, Rate and Wet/Dry controls.
- Local chain and module-control persistence, signal scope and PCM16 WAV export.
- Independent native test target exercises every catalogue entry and invalid-input guards.

