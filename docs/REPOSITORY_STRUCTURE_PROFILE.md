# Repository Structure Profile — VentuSample

**Normative reference:** [Ivan-VHS Repository Structure Standard](https://github.com/Ivan-VHS/DAK/blob/main/docs/REPOSITORY_STRUCTURE_STANDARD.md).

This profile maps the shared structure to VentuSample's offline C++20 audio/DSP application. The sample, DSP, preset and pack contracts remain domain-specific; generic mathematical or server directories are not required.

## Canonical layout

| Concern | Existing canonical location | Responsibility |
|---|---|---|
| Public project entry | `README*.md`, `CONTRIBUTING.md`, `SECURITY.md`, `LICENSE` | Product identity, offline scope, build and contribution/security policy |
| Architecture and guides | `docs/README.md`, `docs/ARCHITECTURE.md`, `docs/QUANTUM_AUDIO_ENGINE.md`, `docs/USER_GUIDE_RU.md` | Signal flow, DSP behavior, user guidance and maturity |
| Native implementation | `src/` | C++20 application, deterministic generator, offline DSP, UI and audio services |
| Compatibility layer | `src/compat/pytyliki-dsp/` | Owned compatibility routines with separately tested C interfaces |
| Build integration | `CMakeLists.txt` | Native targets, optional Qt 6 desktop target and platform configuration |
| Verification | `tests/`, `examples/` | CTest/native tests, compatibility fixtures, pack examples and regression coverage |
| Machine contracts | `schemas/`, `federation/`, root project contracts | Versioned pack/preset/release formats and Reaper exchange contracts |
| Reproducible tooling | `scripts/`, `pyproject.toml` | Audits, metadata checks and maintenance tasks; not the audio engine |
| Supporting material | `assets/`, `examples/` | Licensed creative assets, sample packs and reproducible inputs |
| CI | `.github/workflows/` | Native build, DSP tests, package validation, release and documentation gates |

## Domain separation

- Core audio generation and DSP behavior remain native C++20/C implementation concerns; documentation and presets do not replace runtime evidence.
- Rendering stays offline and deterministic under the declared seed/configuration. Tests should cover boundary values, malformed input, clipping/peak limits and stable output metadata.
- Compatibility code is separately identified and tested. It must not silently fork the semantics of its upstream contract.
- Audio assets and imported material require provenance, licensing and bounded parsing. Do not treat a manifest as proof that its referenced audio is safe or present.
- Reaper exchange remains a versioned, read-only consumer contract unless a future reviewed interface explicitly changes that boundary.

## Formal evidence

The current evidence model is native unit/integration tests, exact schema checks, deterministic fixtures and measured audio outputs. A proof-assistant directory is not part of this product's required layout.

## Change rule

A source move must update CMake targets, DSP test registration, compatibility paths, schema examples, pack generation/validation, documentation links and release workflows together.
