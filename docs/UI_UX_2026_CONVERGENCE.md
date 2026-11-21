# UI/UX 2026 — shared product interface contract

**Maintainer:** Ivan-VHS · **Product:** VentuSample  
**Design language:** AETHER-ANAS / Anatidae — consistent white/blue/red identity adapted to native Qt 6 Widgets behavior.

This is an operational audio workstation, not a marketing dashboard. Keep the generator path clear, expose parameter values and units, and let the render worker operate without visual work entering the audio/DSP path.

## Semantic palette

| Token | Light | Dark | Role |
|---|---|---|---|
| bg | #F4F8FD | #071A2D | Workspace |
| surface | #FFFFFF | #0F2A45 | Panels and editors |
| elevated | #EAF1FA | #173A5E | Secondary panels |
| fg | #122C45 | #E8EDF2 | Primary text |
| secondary | #314B66 | #D0DCE8 | Supporting text |
| muted | #4D6177 | #B3C4D6 | Metadata |
| blue | #004E98 | #70A5FF | Primary actions/navigation |
| blue_hover | #1A5FDB | #8AB6FF | Hover and keyboard focus |
| on_blue | #FFFFFF | #071A2D | Text on blue |
| red | #B5122D | #FF7B86 | Critical/destructive states |
| red_hover | #9C1026 | #FF9AA2 | Critical-action hover |
| on_red | #FFFFFF | #071A2D | Text on red |
| border | #CAD8E8 | #31516F | Boundaries |
| focus | #1A5FDB | #8AB6FF | Visible keyboard focus |
| selection | #D9E8FF | #244F82 | Selected content |
| selection_fg | #122C45 | #FFFFFF | Text on selection |
| disabled | #6B7C90 | #9BAFC4 | Disabled text |

Blue is the action and focus language. Red is reserved for critical and destructive states; do not use red for ordinary keyboard focus. Opaque panels keep parameter surfaces readable.

## Hierarchy and density

- Follow the generator sequence: sound identity → structure → groove/texture → master → optional source → export.
- Keep the generator scrollable. Separate files/results and logs from parameter controls.
- Use an 8 px spacing rhythm, persistent labels, explicit units and readable numeric values.
- Prefer 40–44 px action targets and 38–44 px inputs. Keep accessible names on sliders and parameter controls.
- Show busy, complete and error states in text as well as visual styling. Prevent duplicate render actions.

## Accessibility and input

- Associate visible labels with their fields and expose meaningful accessible names/descriptions.
- Preserve keyboard traversal, focus indication, native text scaling and device-independent layout.
- Target 4.5:1 contrast for normal text and 3:1 for visible focus; aim for 7:1 body text where practical.
- Reduced Motion and Eco suspend the beat indicator and override Expressive Motion. Respect OS reduced-motion preference where available.
- Never signal clipping, failure or success through color alone.

## Motion 2026

| Event | Timing |
|---|---:|
| Direct acknowledgement | 0–40 ms |
| Hover/focus/state | 80–140 ms |
| Pane transition | 160–240 ms |
| Context transition | 260–380 ms |
| Reduced-motion replacement | ≤80 ms |

Expressive Motion may scale supported transitions by 1.35×, capped at 380 ms. Reduced Motion removes spatial movement and caps retained fades at 80 ms; Eco follows the same conservative path. Animations and beat indicators run on the GUI thread, never in DSP callbacks, audio buffers or generator workers. BPM indication is driven by the current project tempo, not an external DAW transport.

## Responsive and high-DPI

Use Qt device-independent sizing and reflow/scroll instead of hiding fields. Validate narrow/wide windows, keyboard-only operation and 100%, 125%, 150% and 200% scaling. Manual display review remains pending until performed.

## Product-specific implementation

VentuSample is a Qt 6 audio-pack workstation. Keep the generator workflow and scrollable form; use project BPM only for the existing restrained beat indicator; keep progress/status separate from logs and results. Give controls accessible names and explicit units. Qt motion remains on the GUI thread and never enters DSP callbacks, audio buffers or generator workers.

## Engineering references

- [Qt 6 Accessibility](https://doc.qt.io/qt-6/accessible.html) · [Qt 6 High DPI](https://doc.qt.io/qt-6/highdpi.html)
- [WCAG 2.2 Focus Appearance](https://www.w3.org/WAI/WCAG22/Understanding/focus-appearance.html)
- [Microsoft Windows app design](https://learn.microsoft.com/windows/apps/design/)

These are targets; this page does not claim manual acceptance checks have already passed.
