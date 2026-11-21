# VentuSample UI motion contract

VentuSample implements the AETHER-ANAS white/blue/red motion system with native Qt animations. Movement is state feedback, never ambient decoration. The 2026 Motion Lab concepts are adapted to Qt Widgets; the browser-only mesh, CSS blur and pointer-driven portal layers are not introduced into the audio workstation.

## Timing and physics

- Controller acknowledgement: 0–40 ms; never delay the action.
- Hover/focus/state: 80–140 ms.
- Pane and graph changes: 160–240 ms.
- Scene/preset transitions: 260–380 ms.
- Reduce Motion and Eco mode: retained fades are capped at 80 ms; spatial movement is omitted.

Default easing is `cubic-bezier(0.2, 0.8, 0.2, 1)` (Qt OutCubic). Normal and Expressive modes use a bounded mass-spring-damper solver on the UI thread: semi-implicit integration, a maximum 32 ms frame delta and two substeps. Expressive Motion uses a stronger response and a 1.35x duration multiplier, capped at 380 ms. Reduced/Eco fades remain bounded at 80 ms and take precedence over Expressive.

## Colour semantics and surfaces

- `#0039A6` / `#0B1F3A`: navigation, focus and calm signal state.
- `#D52B1E` / `#B51A10`: generation, mutation, clipping and safety events.
- Clear native cards, restrained edge highlights and selected-tab feedback carry the Liquid Glass reference without sacrificing readable parameter surfaces.
- No large-area flashing, uncontrolled parallax, mouse magnets or decorative rainbow gradients.

## Settings and tempo

The View → Motion Settings dialog persists Reduce Motion, Eco / Low-CPU Visuals, Expressive Motion and Sync to BPM. VentuSample's internal project BPM drives a restrained beat indicator; this is project-tempo feedback, not an external DAW transport connection. Reduce Motion and Eco mode suspend the beat indicator and cap retained transitions at 80 ms.

Qt animations, spring timers and the beat timer run on the UI thread only. They do not run inside DSP callbacks, audio buffers, generator workers or stochastic-routing code.

## Spring solver parameters

| Mode | Stiffness (k) | Damping (c) | Mass (m) | Duration scale |
| --- | ---: | ---: | ---: | ---: |
| Normal | 240 | 22 | 1.00 | 1.00x |
| Expressive | 340 | 20 | 0.85 | 1.35x |
| Reduced / Eco | no spatial spring | no spatial spring | — | maximum 80 ms |

The solver uses semi-implicit Euler integration with two substeps per timer tick and clamps the frame delta to 32 ms. Its timer is parented to the animated widget, runs on the Qt UI thread, and is stopped when a newer spring replaces it or the widget is destroyed. Existing graphics effects owned by other UI subsystems are never replaced.
