# UI/UX Atlas 2026 — VentuSample

This document translates the Motion Lab and Botanical Atlas references into a production desktop design system. It combines editorial hierarchy, calm atmospheric surfaces, strong navigation, searchable content, clear empty states, accessible dialogs and responsive layouts without importing browser-only effects into native applications.

## Visual direction

The product keeps the Ivan-VHS / AETHER-ANAS semantic identity: blue is navigation and primary action, red is warning / destructive / safety feedback, and neutral surfaces carry the visual atmosphere. Dawn, Sunset and Night change the temperature and depth of surfaces; they do not redefine the meaning of status colors.

| Profile | Page | Surface | Text | Primary |
|---|---|---|---|---|
| Dawn | cool porcelain #F3F6FA | white #FFFFFF | deep navy #10243A | blue #0039A6 |
| Sunset | warm ivory #F5EFE8 | cream #FFF9F2 | charcoal #322B2A | blue #0039A6 |
| Night | deep navy #081522 | slate navy #10263B | near-white #EDF4FA | sky blue #73B7FF |

Red stays reserved for warnings and destructive / safety signals (#B51A10 in light profiles, accessible coral #FF7D78 in Night). Never use red as routine decoration. The legacy Light and Dark preferences remain supported and map to the closest new profile.

## Layout and information hierarchy

- Keep a persistent application identity and a compact global action / appearance area.
- Use a clear page title, one-sentence context and a prominent primary action.
- Group related controls into named cards / sections; put advanced controls behind progressive disclosure.
- Use horizontal tabs or segmented navigation for peer workspaces; do not hide essential actions inside decorative menus.
- Keep primary workflow state visible: selected item, active tab, current file / project, operation state and last result.
- Let content reflow at narrow widths; preserve readable line lengths and usable hit areas rather than shrinking controls.

## Interaction patterns

- Search is scoped to the collection being searched. Preserve the query while filtering, show a specific no-results state, and provide a one-action reset.
- Dialogs have a clear title, concise explanation, explicit primary and cancel actions, Escape dismissal where safe, and focus returned to the invoking control.
- Toast / status feedback is non-blocking and announces completion only after the operation actually completes.
- Progress communicates indeterminate work honestly; never fabricate percentage, health, hardware or audio telemetry.
- Tabs preserve their content state. Selection feedback is immediate; animations never delay a command or data operation.
- Use hover only as a supplement: keyboard focus and selected state must remain equally visible.

## Motion, atmosphere and performance

- Reuse the shared Motion System: bounded spring response, short transitions, persisted preferences and system reduced-motion support where the platform exposes it.
- Reduce Motion removes spatial movement. Eco suppresses nonessential refresh and effects. Both override expressive presentation.
- Do not copy the demo's endless pointer / gyro parallax, floating particles, constant shimmer, blur-heavy glass, click confetti or infinite animation loops into precision desktop workflows.
- Motion belongs to the UI thread only. It must not run in audio callbacks, DSP buffers, workers, benchmarks, device inventory, hashing or build execution.
- Prefer subtle surface elevation, borders, restrained reveal / fade and selected-tab feedback over ambient animation.

## Accessibility and quality gate

- All controls have accessible names; icon-only actions need tooltips / labels.
- Keyboard navigation, visible focus, Escape handling and sensible focus restoration are required.
- Target WCAG 2.2 AA contrast for text and controls. Do not rely on color alone for errors, selected state or progress.
- Prefer 40–44 px controls for frequent pointer actions; do not make essential targets smaller than 24 px.
- Respect OS theme and reduced-motion preferences; persisted app preferences must remain backward-compatible.
- Confirm that search empty states, loading, success, warning, error and disabled states are visually distinct.

## Product adaptation — VentuSample

**Native stack:** C++20 / Qt 6 Widgets desktop audio workstation

Use generator sections as named cards, preserve the Generator / Results & Log / About workspace tabs, and add search scoped to generated output items with a no-match state. Project BPM feedback remains data-driven and distinct from external DAW transport. Keep all visual work outside DSP callbacks, audio buffers, generator workers and stochastic routing.

The reference implementation is a design source, not a claim that every illustrated feature exists in this product. Hardware readings, processing metrics and operation status must always originate from real application data.
