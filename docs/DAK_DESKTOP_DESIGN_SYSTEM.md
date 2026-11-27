# Dak desktop design — VentuSample

Qt 6 Widgets desktop workstation with a scrollable generator and native light/dark appearance. The canonical palette and accessibility rules now live in [UI_UX_2026_CONVERGENCE.md](UI_UX_2026_CONVERGENCE.md).

## Implementation

- Semantic light/dark tokens replace saturated full-blue work surfaces.
- Purple represents primary actions and navigation; mint/teal remains the visible keyboard-focus accent.
- Magenta is reserved for audio-domain signal emphasis; critical/destructive controls remain explicitly labelled and visually separate.
- Opaque, clearly separated cards keep parameter groups and values legible. Glass effects are subtle chrome only and never cover waveform/parameter data.
- Inputs expose accessible names; field labels are linked to controls.
- Preferred target sizes: 40–44 px actions, 38–44 px inputs; numeric sliders remain readable.
- Generator stays scrollable; logs/results and status remain distinct.
- The render worker, DSP callbacks and audio buffers remain independent from all UI animation.

## Acceptance still required

Responsive reflow, keyboard-only traversal, contrast on real displays and physical HiDPI review must be manually checked at 100%, 125%, 150% and 200% before being marked verified. The code being DPI-capable is not proof that every monitor scale has passed review.


## Motion Constitution v2026 precedence

The normative shared motion, layout and telemetry contract is [Motion Constitution v2026](MOTION_CONSTITUTION_V2026.md). This design document supplies visual-language guidance; it does not override the canonical token ranges or product-specific implementation status.

Shared tokens: `--ease-spring`, `--ease-snappy`, `--motion-scale` (0.4–2.2) and normalized `--spring-stiffness` (0.7–1.4). Operating-system reduced-motion and `UI_REDUCED_MOTION` take precedence; nonessential spatial/ambient motion is removed and the underlying state change is retained. Light/dark parity, keyboard focus and high-DPI readability apply to every supported visual state.

The 2×2 overview and focus scene are showcase layout targets. SVG/Canvas or native-paint graphics must be driven by real model values, expose units and textual equivalents, and distinguish unknown, stale and error states. Product-specific `docs/UI_MOTION.md` remains authoritative for shipped native behavior and safety boundaries.


## Shared desktop foundation

The dark variant uses `#050505` canvas and `#101010` work surfaces with white
content and graphite separators. VentuSample keeps its purple action identity,
magenta audio-signal emphasis and mint focus ring. The light variant remains
available for user preference. The shared shell foundation does not recolor
waveforms, meters, plugin artwork or sample content.
