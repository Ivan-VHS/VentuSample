# CINEFORGE — Hollywood Stem & Sample Engine

CINEFORGE is an offline cinematic sample-family generator embedded in VentuSample. It creates coherent scene-driven packs with deterministic seeds, shared tempo/key, dry and wet renders, 24-bit PCM WAV at 48/96 kHz, a combined preview and a JSON manifest.

## MVP

- Scene-to-Pack: scene prompt, BPM, root key, pack size (8–12), seed and micro-take amount.
- Cohesive asset families: sub booms, titan impacts, brass pulses, metal hits, ostinatos, percussion loops, risers, atmospheres and hybrid accents.
- Dry + Wet exports for each family asset; wet versions use deterministic multi-tap spatial reflections.
- Shared tonal reference and deterministic variation seed strategy.
- Combined preview render (first three layers) for audition in the system audio player.
- 24-bit PCM WAV at 48 or 96 kHz; conservative sample-peak headroom.
- Manifest schema: `cineforge.pack.v1`.

## Build

CINEFORGE is compiled into the existing C++20 core and opened from the **CINEFORGE** menu in the Qt 6 desktop app.

```sh
cmake -S . -B build -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The engine is deterministic and offline. The scene prompt is recorded as metadata; the current MVP maps it through the explicit controls and procedural sound-family renderer rather than claiming neural text-to-audio synthesis. Full neural synthesis, transient/body/tail stem isolation, advanced phase analysis, DAW drag-and-drop and cloud generation remain later milestones.
