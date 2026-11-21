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
| Stochastic Router | 8 weighted DSP effects; serial/parallel/hybrid; file/bar/onset triggers |
| Stochastic safety | wet ≤65%, feedback ≤72%, 5–20 ms transition fades, bounded RMS gain, −0.3 dBFS sample peak |
| Stochastic UI | enable/disable, intensity, effect-pool selection, topology and trigger controls |
| Reproducibility | fixed seed + settings + variation produce stable routing decisions |
| Export | PCM16 WAV + manifest v2; v1 compatibility retained |
| Analysis | peak, RMS, crest per file; route names in manifest v2 |
| Key labels | key/mode on pitched material; no fabricated drum key |
| Reaper | v1 manifest metrics/comparison; JSON only, no waveform audit |
| QMR-20 | 20 quality × cohesion × rhythm controls and regression gates |

The stochastic engine is offline DSP; it is not a real-time plugin host. Profiles are original procedural patterns, not artist or recording emulations.
