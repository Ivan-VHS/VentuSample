# Source audio and rights

| Property | Contract |
|---|---|
| Input | local RIFF/WAVE PCM16, mono/stereo |
| Sample rate | 8–192 kHz |
| Limits | ≤128 MiB and ≤20M frames |
| Flow | validate → four slices → beat placement → semitone resample |
| Output | new PCM16 WAV + versioned manifest |

MP3, FLAC, 24-bit and floating-point WAV are unsupported. Resampling changes pitch and playback speed together. The source file is not overwritten; manifest records its filename, not the full local path.

The operator must have rights to import, transform and distribute recordings and third-party assets. SHA-256 identifies bytes; it does not prove ownership, licensing or permission. Local generation does not clear commercial sample rights.

## Optional Reaper language runtime

VentuSample can optionally link Reaper's standalone C17 integer-expression ABI. See [Reaper runtime integration](REAPER_RUNTIME_INTEGRATION.md). The bridge is configured explicitly at build time with `-DREAPER_SOURCE_DIR=/absolute/path/to/Reaper`; it is disabled by default, does not discover or download libraries at runtime, and does not replace the C++20 audio engine or enter the real-time DSP path.
