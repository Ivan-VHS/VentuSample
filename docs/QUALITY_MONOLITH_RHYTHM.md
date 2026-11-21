# VentuSample — Quality, Cohesion and Rhythm Framework

**Framework:** QMR-20  
**Status:** implemented in the offline C++20 renderer  
**Contract compatibility:** keeps `ventusample.pack.v1` unchanged

## Purpose

The supplied 20-row vocabulary is converted into three engineering axes:

1. **Quality** — measurable audio quality, bounded processing, deterministic rendering and honest output claims.
2. **Cohesion** — one consistent product pipeline and one source of truth for a render, without collapsing the code into an unmaintainable monolith.
3. **Rhythm** — a shared musical time grid, controlled swing, humanization, phrase movement and cadence.

“Monolithic” therefore means *coherent end-to-end behavior*, not one giant source file or a loss of module boundaries.

## The 20-point vocabulary matrix

| # | Quality axis | Cohesion axis | Rhythm axis | Operational interpretation |
|---:|---|---|---|---|
| 01 | Качество | Монолитность | Ритмичность | One acceptance path spans validated settings, render, master, export and tests |
| 02 | Качественность | Цельность | Размеренность | Stable bar duration and bounded audio settings |
| 03 | Добротность | Целостность | Пульсация | Consistent transient processing over a stable pulse grid |
| 04 | Добротность исполнения | Единство | Ритмизация | Drum and melodic events share the same swing-time mapping |
| 05 | Безупречность | Нераздельность | Синкопированность | Intentional offbeat placement remains inside the validated meter |
| 06 | Совершенство | Неделимость | Метрика | Musical timing derives from BPM, bar length and a fixed 16-step grid |
| 07 | Мастерство | Сплочённость | Тактовость | Phrase-level events reinforce the bar rather than randomizing the whole arrangement |
| 08 | Профессионализм | Слитность | Метричность | Drum and melodic timing use one subdivision convention |
| 09 | Точность | Консолидация | Периодичность | Seeded patterns repeat reproducibly and phrase turns occur at fixed intervals |
| 10 | Чистота | Стабильность | Цикличность | Stereo-linked bus gain preserves channel balance through compression |
| 11 | Надёжность | Непрерывность | Равномерность | The full requested phrase renders to the expected frame count |
| 12 | Выверенность | Однородность | Мелодичность | Melody accents follow the same swing grid without changing note selection rules |
| 13 | Исключительность | Согласованность | Каденция | Four-bar turnarounds add restrained pickup and fill behavior |
| 14 | Превосходство | Системность | Динамичность | Density controls the intensity of phrase-end activity |
| 15 | Искусность | Компактность | Согласованность | Fill logic is localized and does not duplicate the core event scheduler |
| 16 | Отточенность | Нерасчленённость | Повторяемость | One parameter snapshot and seed govern a reproducible render |
| 17 | Филигранность | Сопряжённость | Текучесть | Swing and micro-humanization complement, rather than replace, the metric grid |
| 18 | Высококлассность | Сбалансированность | Раскачка | Shared stereo gain avoids independent L/R compressor pumping |
| 19 | Основательность | Фундаментальность | Поступательность | The pipeline remains offline, bounded and portable before optional acceleration |
| 20 | Эталонность | Непоколебимость | Ритмика | Regression tests protect duration, determinism, swing response and peak ceiling |

## Implemented behavior

### Billionaire Golden Era profile

The requested luxury / golden-era vocabulary is implemented as one production profile rather than a stack of near-duplicate genre labels:

- **Crate rebuild:** imported four-slice source material is reassembled in a deterministic 0-2-1-3 order before the existing beat-synchronous placement and pitch resampling.
- **Heavy 90s / dragged boom-bap pocket:** the profile uses a five-event kick map and classic backbeat snare anchors. The existing user-controlled swing and snare offset provide the drag; neither is overwritten by profile selection.
- **Premium drum programming:** kick and snare levels are weighted independently while the hat layer is intentionally restrained. Phrase-end ghost snares use a profile-owned probability.
- **MPC / saturation / mastering chain:** the profile is designed to work with the existing MPC 2500/3000XL/5000 color, grit, saturation, filter and stereo-linked master controls. No unsupported tape emulation, LUFS target or true-peak claim is introduced.
- **Translation and export:** the normal PCM16 export, deterministic seed, per-file peak/RMS/crest measurements and −0.3 dBFS sample-peak ceiling remain active.

### Quality

- The stereo bus compressor now uses one detector envelope and one gain value for both channels. A transient on one side no longer drives an independent gain reduction on only that side.
- The mastering stage keeps the existing sample-peak ceiling at approximately −0.3 dBFS.
- The code explicitly labels this as a **sample-peak ceiling**, not an oversampled true-peak limiter.
- Existing finite-value sanitization, bounded parameter validation and per-file peak/RMS/crest measurements remain in place.

### Cohesion

The render contract remains:

`validated Parameters → seeded event schedule → stereo render → sampler color → linked stereo master → measurements → WAV + manifest`

- One immutable parameter object is used throughout a generation request.
- Seed and variation index control procedural choices.
- Manifest v1 retains the exact settings snapshot and output measurements.
- The application remains split into desktop, control, renderer, I/O and verification layers.

### Rhythm

- A shared `swingOffsetForStep` mapping is used for drum events and generated melodic accents.
- Micro-humanization remains a separate, small timing variation; it does not redefine the underlying grid.
- Every fourth bar can receive a restrained turnaround: a pickup kick, a probability-gated ghost snare at higher density, and a short hi-hat fill.
- Existing genre-specific kick/snare profiles and optional triplet rolls remain the foundation.
- Drum-only random decisions are gated off for melody-only and bass-only targets, so irrelevant drum settings do not perturb their seeded note sequence.

## Regression coverage

The native test suite now checks that:

- a four-bar render is non-empty and repeatable for the same seed;
- rendered frame count matches the requested BPM and bar count;
- changing swing from 62% to straight 50% changes the rendered result;
- the master sample peak remains below the established ceiling.

## Compatibility and limitations

- No pack schema migration is required; the exported manifest remains `ventusample.pack.v1`.
- Existing user controls and saved preferences remain compatible.
- There is no claim of LUFS normalization, oversampled true-peak limiting, phase-correlation analysis, tempo-preserving time-stretch, live playback or GPU acceleration.
- Phrase turnarounds are deterministic procedural behavior, not a promise of human-composed arrangement quality.
