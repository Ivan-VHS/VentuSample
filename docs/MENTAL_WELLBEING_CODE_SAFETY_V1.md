# Mental Wellbeing & Depression Awareness v1 — Code Safety Standard

**Policy ID:** `mental-wellbeing-code-safety.v1`  
**Applies to:** VentuSample C++20 audio generation, WAV import/export, DSP, manifests and plugin metadata  
**Status:** Engineering guardrails; not a clinical protocol

## Why this exists

This standard translates the non-stigmatizing, help-seeking and self-care themes in the supplied *Mental Wellbeing & Depression Awareness v1* material into engineering safeguards. The supplied transcript contains only opening and closing segments; its missing middle is not reconstructed. Personal stories, sponsor material, timestamps and the quoted prevalence figure are not clinical evidence or executable rules.

VentuSample is an offline procedural audio workstation, not a mental-health product. It must not infer depression, distress, mood, personality or psychological state from audio, tempo, musical choices, editing activity or generated patterns.

## Non-negotiable safeguards

- **No diagnosis or inference:** do not classify a user's mental state from imported audio, BPM, spectral features, sample selection, arrangement, project history or usage.
- **No medical advice:** do not recommend medication, treatment, dosage, therapy or clinical decisions. Do not frame audio analysis as a validated psychological assessment.
- **Offline by default:** preserve local generation and processing. Do not add audio uploads, remote analysis, hidden microphone capture or background telemetry.
- **Consent and source control:** microphone or recording input, should it ever be introduced, requires a deliberate user action, visible active state and a clear stop control. Never activate capture implicitly.
- **Audio privacy:** imported WAV files and manifests can carry identifying metadata. Do not copy unnecessary source metadata into exports; never log raw audio or private paths by default.
- **Bounded DSP:** preserve sample-rate, channel, frame, file-size, RIFF chunk and allocation limits; test malformed and truncated input.
- **Plugin safety:** manifest discovery remains metadata-only; unreviewed entries stay disabled. Do not enable arbitrary DLL/SO loading or manifest-driven execution.
- **Respectful UX:** generator presets and processing reports describe sound characteristics, not the user's personality, worth, mood or health.
- **Blameless engineering:** bugs in a render or export path are system defects to reproduce and fix, not personal failures.

## VentuSample-specific threat model

| Surface | Required protection |
|---|---|
| WAV import | Validate PCM format, sample rate, channels, frames, file length and RIFF chunk bounds |
| DSP / renderer | Bound allocations and processing work; deterministic seeded behavior; no health inference from features |
| Export / manifests | Minimize identifying metadata; record reproducibility data only when required |
| Plugin discovery | Metadata-only scan; disabled by default; explicit reviewed native handler |
| Reaper bridge | Explicit build-time opt-in; no runtime discovery/download; no audio in the integer-expression bridge |
| Desktop / projects | Offline-first; no hidden microphone, upload, mood tracking or productivity telemetry |

## Change acceptance checklist

A change touching WAV I/O, rendering, DSP, manifests, plugins, federation or persistence is ready only when:

1. Audio input and processing remain bounded, with safe failure on malformed files.
2. No audio, project data or personal identifiers are transmitted or collected in the background.
3. No mental-health or emotional-state classifier is introduced.
4. Plugin metadata cannot trigger native code execution.
5. User-facing text remains factual, non-stigmatizing and technically precise.
6. Tests cover truncated chunks, invalid lengths, overflow, channel/rate limits and deterministic output.

## Automated enforcement

`mental-wellbeing-code-safety.v1.json` is the machine-readable policy contract.  
`scripts/check_mental_wellbeing_guardrails.py` validates required prohibitions, VentuSample-specific offline/audio/plugin boundaries and this document. The dedicated GitHub Actions workflow runs the validator on pushes and pull requests. This gate does not replace C++ tests, sanitizers, fuzzing or privacy review.

## Scope and limitations

This standard is not diagnosis, screening, counselling, crisis response or treatment guidance. Audio analysis and musical features must never be marketed or presented as a substitute for a qualified healthcare professional.
