# Engineering Baseline

**Status:** normative  
**Product:** VentuSample offline sample-pack workstation

## Product boundary

VentuSample is a local, offline generator for procedural hip-hop sample packs. It owns pattern generation, rendering, sample slicing, PCM conversion, pack export and the desktop workflow. It is not a DAW, plugin host, real-time audio engine, recording utility, sample marketplace or audio-device manager.

## Input and output contract

- Imported audio MUST be RIFF/WAVE PCM16, mono or stereo, with a supported sample rate from 8 kHz to 192 kHz.
- Input files MUST be size- and frame-bounded before allocation and parsing. The current product limit is 128 MiB and 20 million frames.
- Unsupported encodings MUST be rejected explicitly. They MUST NOT be guessed, silently converted or partially accepted.
- Imported source audio MUST remain local. The generator MUST NOT upload, transmit or rewrite the source file.
- Output MUST use stereo RIFF/WAVE PCM signed 16-bit little-endian. The versioned `ventusample.pack.v1` manifest MUST describe the effective settings and per-file measurements.
- Existing output artifacts MUST NOT be silently overwritten.
- Transposition by linear resampling changes both pitch and duration; it MUST NOT be described as tempo-preserving time stretch.

## Determinism and resource control

- Parameter ranges, sample-rate choices, bar counts and variation counts MUST be validated before rendering.
- A fixed seed MUST produce repeatable output for the same effective parameter snapshot and software version.
- Rendering MUST use bounded local buffers and a controlled worker job. The user interface MUST remain responsive while a pack is generated.
- The renderer MUST report errors explicitly and MUST NOT present an incomplete pack as successful output.
- Peak, RMS and crest measurements are descriptive measurements; a sample-peak ceiling MUST NOT be described as a true-peak limiter.

## Musical profiles

Genre profiles are procedural starting points. They MUST NOT be represented as recordings, MIDI transcriptions or exact emulations of named artists, labels, samplers or records. Sampler character controls are approximations, not bit-exact hardware models.

## Integration and verification

- Optional integrations MUST be explicit, versioned and independently tested. A documented sibling-project relationship MUST NOT imply that its runtime or binary is embedded.
- The checked-in C++ implementation, JSON schema and regression tests define shipped behavior.
- Planned features, proposed interfaces and design references MUST be labelled separately from implemented capabilities.
