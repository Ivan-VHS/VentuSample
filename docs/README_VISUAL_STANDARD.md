# README and Visual Documentation Standard

**Scope:** VentuSample README family, documentation indexes, audio diagrams, screenshots and visual assets. Source, tests, manifests and schemas remain authoritative.

## Publishing principles

- Lead with an accurate description of the offline workstation and its primary user task.
- Keep the light/dark hero, current CI/license badges and quick start near the top.
- Distinguish deterministic sample generation, offline DSP and the optional Qt desktop shell.
- Never describe this project as a DAW, real-time audio engine, VST/AU host or cloud service.
- A docs refresh is not a product release or version claim.

## README sequence

Adaptive hero → one-line pitch → badges/navigation → product boundary → requirements/quick start → documentation-revival comparison → capability matrix → visual overview → Mermaid data path → real CMake options/trust boundaries → repository map → verification → contribution/license.

## Hero and assets

Use `assets/readme/hero-banner-light.svg` and `assets/readme/hero-banner-dark.svg` with equal intrinsic dimensions and viewBox. Reference them with `<picture>` and `prefers-color-scheme`. SVGs must be self-contained and provide accessible fallback text.

## Screenshot and diagram rules

- Link real desktop captures to their committed original image.
- Caption the screenshot separately from the workflow/architecture illustration.
- Use a two-column, one-row visual grid with descriptive alt text.
- Do not call the workflow illustration a UI screenshot.
- Do not publish fake meters, output waveforms or fabricated performance values.
- If no verified capture exists, state that explicitly and use labelled schematics.

## Technical accuracy

- Check CMake 3.21+, C++20, Qt 6 and core-only command examples against current build options.
- State that rendering and DSP are offline; do not imply playback or a real-time device callback.
- Treat Quantum Audio Engine names as creative DSP module identities, not quantum hardware.
- State plugin manifest discovery is metadata-only unless a reviewed handler is registered.
- Document the optional Reaper bridge as a separate integration, not the DSP engine.
- Do not invent environment variables, user accounts or hosted APIs.

## Localization and acceptance

Keep English, Ukrainian, Russian and Simplified Chinese README variants aligned in formats, limits, CMake flags and feature maturity. Check image paths, Mermaid syntax, links, code blocks and `git diff --check`.
