# Anatidae desktop design — VentuSample

Qt 6 Widgets desktop workstation with a scrollable generator and native light/dark appearance. The canonical palette and accessibility rules now live in [UI_UX_2026_CONVERGENCE.md](UI_UX_2026_CONVERGENCE.md).

## Implementation

- Semantic light/dark tokens replace saturated full-blue work surfaces.
- Blue represents primary actions, navigation and visible keyboard focus.
- Red is reserved for critical or destructive states.
- Opaque, clearly separated cards keep parameter groups and values legible.
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
