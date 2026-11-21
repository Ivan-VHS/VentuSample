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
