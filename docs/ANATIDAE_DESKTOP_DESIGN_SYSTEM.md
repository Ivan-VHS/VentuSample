# Anatidae Desktop Design System — VentuSample

**Status:** normative design contract for the native C++20 / Qt 6 workstation. It consolidates the current visual direction and defines the guardrails for future interface work; it does not claim unimplemented behavior.

## 1. Product intent

VentuSample is an offline procedural sample-pack workstation. The interface should make musical configuration, deterministic variation, rendering progress and export evidence easy to inspect. Sound profiles and parameters are the product's primary vocabulary. The Anatidae identity is a restrained companion layer, never a substitute for musical or engineering terms.

## 2. White-blue-red semantic palette

| Semantic token | Light theme | Dark theme |
|---|---|---|
| Workspace | `#F4F8FD` | `#071A2D` |
| Surface | `#FFFFFF` | `#0F2A45` |
| Elevated surface | `#EAF1FA` | `#173A5E` |
| Primary text | `#122C45` | `#E8EDF2` |
| Secondary text | `#314B66` | `#D0DCE8` |
| Muted text | `#4D6177` | `#B3C4D6` |
| Primary blue | `#004E98` | `#70A5FF` |
| Blue hover | `#1A5FDB` | `#8AB6FF` |
| Action / danger red | `#B5122D` | `#FF7B86` |
| Red hover | `#9C1026` | `#FF9AA2` |
| Border | `#CAD8E8` | `#31516F` |
| Focus | `#1A5FDB` | `#8AB6FF` |
| Selection | `#D9E8FF` | `#244F82` |

The values match the current Qt stylesheet. Do not introduce unrelated accent families. Success, warning and failure must remain distinguishable by text and shape as well as color. Review body text and controls against WCAG AA contrast; never shrink text to fit a dense parameter surface.

## 3. Layout and controls

- Keep the brand/header, short product explanation and offline/export state visibly separate.
- Group musical configuration (style, target, key, scale, sampler, timbre and sample rate) separately from timing, variation, source-sample and mastering controls.
- Every slider has a persistent label and a visible numeric value. Numeric values and units remain independently readable.
- Primary generation and folder-opening actions must not be visually conflated. Generation requires explicit confirmation and clearly identifies the output destination.
- Keep the file list and render log inspectable. Progress and completion status must remain visible without requiring log inspection.
- Preserve scrollability and usable hit areas at the current minimum supported window size; future responsive layouts must be tested before changing that minimum.

## 4. State, motion and accessibility

- Rendering progress, success and failure use explicit text. A progress animation must not be the only indication that a job is running.
- The worker-thread renderer must not block ordinary window interaction. Closing the app must keep its current controlled worker shutdown behavior.
- Theme changes must repaint the whole surface consistently and remain readable in both light and dark modes.
- Keyboard traversal, visible focus, labels associated with inputs and readable slider values are required. Reduced-motion preferences must be respected by any future animation.
- Do not use a decorative mascot, quack, feather or waterfowl symbol as the only accessible name or status.

## 5. Engineering and content boundaries

- Source audio remains local and is validated as bounded PCM16 WAV input.
- Generated output remains PCM16 stereo WAV plus the versioned manifest.
- Existing artifacts are not silently overwritten; generation requires an explicit user confirmation.
- Fixed-seed repeatability, parameter bounds, sample-rate bounds and explicit render errors remain product requirements.
- Genre profiles remain procedural starting points and must not be presented as copied recordings or exact artist/hardware emulations.
- The current source, manifest schema and regression tests define shipped behavior.

## 6. Anatidae naming and companion layer

The shared vocabulary and guardrails are defined in [the federation naming canon](../federation/ANATIDAE_NAMING_CANON.md). For VentuSample, **Утёнок** may be used for optional first-pack guidance and **Крякуша** for non-critical help around the existing variation workflow. **Пёрышко** may serve as a small file-summary decoration only while filenames, PCM format, sample rate, key, scale, seed and measurements remain explicit.

Do not add a genre, sound profile, sample format, generator mode or audio behavior merely to consume a vocabulary word. Existing style names and DSP parameters remain the canonical musical vocabulary. Keep confirmation before pack generation, bounded input validation, local-only processing, explicit errors and the −0.3 dBFS sample-peak statement unchanged.

The Anatidae layer must use the existing white-blue-red visual language and the application's current theme behavior. Feather/wing details are secondary; they must not reduce the contrast or readability of controls, numeric parameters, progress or render status. Reduced-motion behavior and worker-thread responsiveness remain mandatory.

## 7. Release review

- [ ] Both themes use the documented semantic palette.
- [ ] Parameters remain labeled and values remain visible.
- [ ] Generation confirmation identifies the output location.
- [ ] Busy, success and failure states are readable without relying on color or animation.
- [ ] Worker-thread responsiveness and orderly shutdown are preserved.
- [ ] Source import and output limits are communicated accurately.
- [ ] Optional Anatidae copy does not replace technical labels or imply new behavior.
- [ ] Keyboard focus, traversal and reduced-motion behavior are checked.
