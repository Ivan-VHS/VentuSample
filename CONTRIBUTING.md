# Contributing to VentuSample

Thank you for helping improve VentuSample. The project is an offline C++20 sample workstation with a deterministic renderer, bounded WAV processing, and an optional Qt 6 desktop interface.

## Preserve the project boundary

The renderer must remain offline and reproducible for fixed inputs and seed. Do not add network calls, telemetry, audio-device playback, DAW hosting, or hidden background services. Preview renders must remain clearly distinct from final renders. Any option that changes WAV bytes, metadata, sample count, or normalization behavior must be explicit and documented.

## Before opening a pull request

1. Read README.md and the relevant audio, format, and quality documents under docs/.
2. Keep DSP and file parsing in the portable core where practical; keep Qt responsible for presentation.
3. Add deterministic tests with fixed seeds and small fixtures for renderer and WAV changes.
4. Update README.md, README.uk.md, README.ru.md, and README.zh-CN.md when user-visible controls, supported ranges, profiles, or examples change.
5. Update the pack schema and contract when a manifest or exported format changes.
6. Preserve input-size, frame-count, and path limits; reject malformed data rather than guessing.

## Local checks

    cmake -S . -B build -DBUILD_TESTING=ON
    cmake --build build --parallel
    ctest --test-dir build --output-on-failure
    python .github/scripts/check_readme_parity.py

Run the Python contract validators and test scripts used by the CI workflow when schemas, manifests, or documentation contracts change. Use the same Qt availability assumptions as CI when checking the desktop application.

## Pull request expectations

Explain the creative or engineering task, the exact parameters affected, deterministic behavior, file-format compatibility, tests run, and any known limitations. Include before/after evidence only when it can be reproduced from fixed inputs.

## Security and review

Treat presets, manifests, WAV chunks, paths, and imported files as untrusted input. Keep parsers bounded. Do not claim that a hash proves authorship or rights to source material. Keep changes focused and avoid unrelated generated audio or binary artifacts.
