# Anatidae desktop design — VentuSample

Qt 6 light/dark theme and scrollable generator are implemented. Responsive reflow, keyboard/focus regression and physical HiDPI review remain acceptance work until tested.

## Semantic palette

| Token | Light | Dark |
|---|---|---|
| Workspace | #F4F8FD | #071A2D |
| Surface | #FFFFFF | #0F2A45 |
| Controls | #EAF1FA | #173A5E |
| Primary text | #122C45 | #E8EDF2 |
| Secondary | #314B66 | #D0DCE8 |
| Muted | #4D6177 | #B3C4D6 |
| Blue | #004E98 | #70A5FF |
| Blue focus | #1A5FDB | #8AB6FF |
| Red | #B5122D | #FF7B86 |
| Red hover | #9C1026 | #FF9AA2 |
| Border | #CAD8E8 | #31516F |
| Selection | #D9E8FF | #244F82 |

No teal/amber/green/violet brand accents. Status is text/icon plus color; target WCAG AA for actionable controls and AAA for body text where achievable.

## Layout and interaction

1. Sound identity → 2. grid/structure → 3. groove/texture → 4. master/motion → 5. optional source → 6. export.
- 8px spacing rhythm; 8–12px radii; 44px preferred action targets.
- Persistent labels and units; opaque surfaces; generator stays scrollable.
- While rendering, prevent duplicate requests and keep UI responsive.
- Brand imagery stays in chrome/empty states, never behind controls.
- UI never implies live playback, audio capture or online telemetry.

Duck vocabulary is an internal design map for clear entry, grouping and restrained geometry—not mandatory mascots or novelty labels.
