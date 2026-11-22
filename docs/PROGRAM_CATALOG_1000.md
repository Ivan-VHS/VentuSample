# 1,000-program adoption map: VentuSample

This file assigns the sound, music, sample, effects and audio-analysis portion of the supplied 1,000-item catalogue. VentuSample is the sole owner of sound-processing modules across Ivan-VHS projects.

## Ownership within the catalogue

| Catalogue IDs | Original subject area | VentuSample treatment |
|---|---|---|
| 121–170 | DSP and audio analysis originally grouped under Pytyliki | **Reassigned to VentuSample** by the repository ownership rule |
| 701–750 | DAWs, samplers, sequencers, DJ and plugin hosts | Offline generator, sample workflow, project and export formats |
| 751–830 | Synthesizers, samplers and drum machines | Original procedural voices, envelopes, sample zones, kits and pattern workflows |
| 831–880 | Audio frameworks, music languages and I/O libraries | C11/C++20 offline processing, MIDI/notation/data adapters; no real-time host by default |
| 881–940 | Effects, mastering, stem separation and time stretch | Original offline processors and bounded analysis; optional model integrations require separate review |
| 941–1000 | Sample catalogs, generative systems, notation and music analysis | Metadata search, spectral descriptors, tags, MIDI/notation and local analysis |

## First implementation wave

- **Offline Spectrum Analysis** — pure C11 radix-2 FFT with a Hann window, caller-owned output bins, peak frequency/magnitude, spectral centroid and 85% rolloff.
- FFT size is limited to powers of two from 256 to 65,536. The analyzer reads an existing in-memory buffer and never opens a device or modifies samples.
- **Linear Sample-Rate Conversion** — a bounded C11 preview-quality resampler for mono/stereo in-memory buffers from 8–192 kHz. It uses deterministic linear interpolation and explicitly does not claim band-limited production quality.
- **Standard MIDI File Inspection** — a C11 parser for SMF format 0/1 with note events, tempo, time signature, program changes and running status. The parser is bounded, read-only and has no device playback path.

## Adoption rules

1. All sound source, DSP, WAV analysis, sample catalog, MIDI and audio integration modules live here, not in Pytyliki, Reaper or OPlague.
2. No copied proprietary plugin, DAW, sample-library or GPL implementation code. Implement original algorithms or use independently approved compatible dependencies.
3. Real-time audio callbacks, VST/AU hosting, device enumeration and network transport are separate product decisions; the first wave remains offline and deterministic.
4. Source sample provenance and license fields must remain visible. A hash proves identity, not ownership or usage rights.
5. Generative/ML services are not required for local operation. Cloud generation and external stem services remain optional adapters, never hidden dependencies.
6. All DSP paths validate frame counts, rates, channels, finite values and memory bounds before processing.

## Remaining backlog themes

The 121–170 cluster prioritizes FFT alternatives, resampling, codecs, loudness and audio feature extraction. The 701–750 cluster prioritizes import/export formats, timeline/pattern editing and plugin interchange. The 751–830 cluster prioritizes original synthesis engines, sampler zones, velocity layers, sequencers and instrument metadata. The 831–880 cluster prioritizes MIDI/OSC, audio graph descriptions and portable framework adapters. The 881–940 cluster prioritizes EQ/dynamics, time stretch, convolution, stem separation and spatial tools. The 941–1000 cluster prioritizes searchable metadata, pitch/tempo/chord analysis, MIDI transcription and notation export.

This roadmap does not claim a real-time DAW or third-party plugin host.
