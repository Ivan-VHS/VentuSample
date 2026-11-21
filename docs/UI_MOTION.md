# VentuSample UI motion contract

VentuSample implements the AETHER-ANAS white/blue/red motion system with native Qt animations. Movement is state feedback, never ambient decoration.

## Timing

- Controller acknowledgement: 0–40 ms; never delay the action.
- Hover/focus/state: 80–140 ms.
- Pane and graph changes: 160–240 ms.
- Scene/preset transitions: 260–380 ms.
- Reduce Motion and Eco mode: retained transitions are capped at 80 ms; spatial movement is omitted.

Default easing is `cubic-bezier(0.2, 0.8, 0.2, 1)` (Qt OutCubic); transient feedback uses a short, damped opacity pulse. Linear motion is reserved for time/phase indicators.

## Colour semantics

- `#0039A6` / `#0B1F3A`: navigation, focus and calm signal state.
- `#D52B1E` / `#B51A10`: generation, mutation, clipping and safety events.
- No large-area flashing, uncontrolled parallax or decorative rainbow gradients.

## Settings and tempo

The View → Motion Settings dialog persists Reduce Motion, Eco / Low-CPU Visuals and Sync to BPM. VentuSample's internal project BPM drives a restrained beat indicator; this is project-tempo feedback, not an external DAW transport connection. Reduce Motion and Eco mode suspend the beat indicator and cap retained transitions at 80 ms.

Qt animations and the beat timer run on the UI thread only. They do not run inside DSP callbacks, audio buffers, generator workers or stochastic-routing code.
