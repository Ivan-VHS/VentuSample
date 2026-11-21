# Motion Constitution v2026

> Shared motion, interaction-graphics and documentation contract for the Ivan-VHS desktop ecosystem.

**Version:** 2026.1 · **Canonical maintainer:** Ivan-VHS  
**Canonical upstream:** [Pytyliki / Motion Constitution v2026](https://github.com/Ivan-VHS/Pytyliki/blob/main/docs/MOTION_CONSTITUTION_V2026.md)  
**Status:** normative design contract. Product runtime contracts define what is actually shipped.

This file is mirrored across the four repositories so each checkout remains self-documenting. The Pytyliki copy is canonical upstream. Mirrors must stay synchronized. A product-specific `docs/UI_MOTION.md` may narrow a rule for its native toolkit, but must not contradict the accessibility or safety invariants below.

## 1. Five invariants

1. **One token vocabulary.** Timing, easing, motion scale, spring response, theme roles and domain accents use the names below. Native applications map them to their toolkit; CSS variables do not execute inside Win32, Tk or Qt.
2. **Accessibility wins.** OS reduced-motion preferences and `UI_REDUCED_MOTION` override application presets. Reduced motion removes spatial/ambient movement and keeps essential acknowledgement immediate or at most 80 ms. The state change itself is never suppressed.
3. **Specification before feature.** Every motion or graphic declares its trigger, state source, token/physics, reduced-motion path, theme behavior, data units and maturity before implementation.
4. **Graphics represent real state.** A ring, waveform, chart, AST panel or progress stream is driven by the real application model, includes a readable textual/numeric equivalent and exposes stale/error/unknown states. No invented telemetry.
5. **Documentation is a contract.** Root and localized READMEs, feature matrix, motion contract, UI implementation and screenshots must agree. Mark every capability implemented, partial, planned or unavailable.

## 2. Canonical motion tokens

The names are normative for CSS showcases and equivalent native constants:

| Token | Canonical value | Purpose |
|---|---|---|
| `--ease-spring` | `cubic-bezier(0.22, 1.18, 0.36, 1)` | short damped acknowledgement; clamp overshoot to legal property bounds |
| `--ease-snappy` | `cubic-bezier(0.20, 0.80, 0.20, 1)` | navigation, pane and state transitions |
| `--ease-water` | `cubic-bezier(0.37, 0, 0.63, 1)` | optional showcase artwork only |
| `--motion-scale` | `1.0`, bounded to `0.4–2.2` | user-adjustable duration multiplier |
| `--spring-stiffness` | `1.0`, bounded to `0.7–1.4` | normalized multiplier for reviewed spring preset |
| `--spring-damping-ratio` | `1.0` | normalized damping target; preserve stability when stiffness changes |

Base durations: `instant=0 ms`, `micro=100 ms`, `fast=140 ms`, `standard=180 ms`, `spatial=220 ms`, `emphasis=320 ms`. Apply scale only to supported transitions. A product may retain a tighter hard cap where precision, safety or platform behavior requires it.

For a mass-spring-damper solver, `k_effective = k_base × stiffness`. Preserve the damping ratio when stiffness changes: `c = 2 × ζ × sqrt(k_effective × m)`. Clamp inputs, use finite values, cap frame delta, integrate with bounded substeps, snap when settled and cancel work when the owner is destroyed. Never run the solver in a product-work worker.

A CSS showcase may map the tokens as follows:

```css
:root {
  --ease-spring: cubic-bezier(0.22, 1.18, 0.36, 1);
  --ease-snappy: cubic-bezier(0.20, 0.80, 0.20, 1);
  --ease-water: cubic-bezier(0.37, 0, 0.63, 1);
  --motion-scale: 1;
  --spring-stiffness: 1;
  --spring-damping-ratio: 1;
  --duration-fast: 140ms;
  --duration-standard: 180ms;
  --duration-spatial: 220ms;
}
.panel {
  transition:
    opacity calc(var(--duration-standard) * var(--motion-scale)) var(--ease-snappy),
    transform calc(var(--duration-spatial) * var(--motion-scale)) var(--ease-spring);
}
@media (prefers-reduced-motion: reduce) {
  .panel { transition-duration: 0.01ms; animation-duration: 0.01ms; scroll-behavior: auto; }
}
```

Clamp externally supplied scale values to `0.4–2.2`; never trust raw configuration input. Native clients map the same values to toolkit timing/preferences and also honor the operating-system setting.

## 3. Accessibility, themes and interaction

- Support light/dark themes with semantic roles for canvas, surface, elevated surface, primary/secondary text, border, focus, selection, success, warning and danger.
- Never encode status by colour alone. Preserve visible keyboard focus, accessible names, labels, units and native keyboard behavior.
- Motion must not delay a command, device scan, file operation, debugger action, render or export.
- Reduced Motion removes parallax, auto-pulsing, continuous beat indicators, ripples, particles, portal motion and spatial transforms. Essential selection/status feedback may change immediately; any retained fade is at most 80 ms.
- Eco / Low-CPU mode suppresses ambient animation and optional refresh work, not telemetry required for safety or correctness.
- OS settings and `UI_REDUCED_MOTION` outrank stored preferences. A user preference never overrides a stronger system preference.
- Animation is UI-thread-only, bounded, cancellable and disposed with its widget. It must not enter DSP callbacks/audio buffers, processor benchmarks, driver/package scans, compiler/runtime execution or worker queues.

## 4. Lab layout: 2×2 and focus scene

- **2×2 overview:** four equal domain panels (IDE & AI workspace, drivers & peripherals, DAW & mixer, processors & telemetry). Each panel has a title, one primary live state, one visual sample and a textual value/status.
- **Focus scene:** the selected domain expands into the main stage; the other three remain compact, labelled navigation tiles. Keyboard users can change focus without drag or animation.

Layout is presentation state only. It must not change device configuration, project settings, render parameters or processor behavior. Reflow at narrow widths rather than compressing labels or hiding critical actions. With reduced motion, switch immediately while preserving focus and selection.

## 5. SVG and Canvas telemetry

Use SVG for vector indicators, icons, static curves and ring progress; use Canvas/native paint for dense or frequently sampled traces. Every visualization must:

- bind to a named model value and unit, with timestamp/source where relevant;
- display a text/numeric equivalent and distinguish zero, unavailable, stale and error;
- use bounded sampling/decimation and pause or lower decorative refresh when hidden;
- remain legible in light/dark themes and high DPI;
- have a non-animated reduced-motion form and never rely on colour alone;
- draw on the UI thread while collection remains in its existing safe model/worker path.

A ring reflects actual operation state; a waveform uses actual audio samples or is labelled **demo data**; a curve plots sourced values with units. Never invent USB connection, OTA percentage, driver firmware status, pointer DPI/polling, CPU voltage, temperature, frequency or DAW level.

## 6. Domain matrix and truthful product mapping

| Domain | Intended visual language | Current repository mapping | Explicit boundary |
|---|---|---|---|
| **IDE & AI workspace** | command palette, editor/AST fold, diagnostics, diff timeline, tab focus | **Reaper:** native IDE, project tree/editor, local language tooling, CODEX-66 browser, LSP/DAP and debugger | no cloud AI agent, streamed AI diff or cross-file semantic inference unless implemented and tested |
| **Drivers & peripherals** | device state, profile/status cards, package verification and (if supported) OTA progress | **OPlague:** read-only PnP inventory, INF/ZIP inspection, hashes and candidate compatibility graph | no driver install/load/modify, firmware OTA, device DPI/polling controls or live USB animation |
| **DAW & mixer** | transport, bus/meter levels, waveform and parameter transitions | **VentuSample:** Qt generator, offline DSP/plugin lab, project BPM indicator and WAV export | not a real-time DAW, external transport link, VST/AU host or live mixer; no DSP callback animation |
| **Processors & telemetry** | measured frequency/temperature charts, bounded V/F reference curve and workload flow | **Pytyliki:** processor discovery, bounded benchmarks/planning, static diagnostics and local DSP | no overclock/voltage writes or live thermal/V/F control unless a reviewed module and sensor source are added |

Domain names define shared showcase vocabulary; they do not imply every product implements every feature. Product source, schemas and tests remain authoritative for shipped behavior.

## 7. Feature-state contract

- **Implemented:** code path and automated/manual acceptance evidence exist.
- **Partial:** a narrower native behavior exists; list the exact subset.
- **Planned:** intent only; keep it out of current capability claims and live telemetry.
- **Unavailable:** unsupported by product architecture or explicitly prohibited.

Every feature record declares: stable ID/owner; trigger and authoritative state source; motion token, duration and spring parameters; Reduced Motion and Eco behavior; theme/layout behavior; graphic source, units, cadence and stale/error semantics; UI-thread ownership and cancellation; maturity and evidence.

## 8. Synchronization and acceptance

When a feature changes, update in the same change set: (1) canonical constitution and mirrors only if the shared contract changes; (2) product `docs/UI_MOTION.md`; (3) feature/architecture docs if model behavior changes; (4) root/localized READMEs and `docs/README.md`; (5) in-app graphic only when backed by a real model value; (6) screenshots/build artifacts only after a fresh real application capture.

Minimum acceptance matrix: scale `1.0×`, `0.4×`, `2.2×`; Reduce Motion on/off; Eco on/off; light/dark; keyboard-only focus; 2×2/focus scene; resize/high DPI; stale/error/unknown telemetry. Record checks not run as pending, never passed.

### Copy-ready feature record

```yaml
motion_contract: Motion Constitution v2026
feature_id: domain.component.action
owner: product/module
trigger: explicit user action or model event
state_source: canonical model field and unit
motion:
  easing: --ease-snappy
  duration_token: standard
  motion_scale: 1.0
  spring_stiffness: 1.0
  damping_ratio: 1.0
reduced_motion: immediate state change; no spatial/ambient movement
eco: suppress nonessential refresh only
themes: [light, dark]
layouts: [2x2, focus-scene]
telemetry:
  source: real model value
  unit: explicit
  stale_after_ms: product-defined
  error_state: explicit
maturity: planned
evidence: []
```

## 9. Product implementation references

- [Reaper — IDE motion contract](https://github.com/Ivan-VHS/Reaper/blob/main/docs/UI_MOTION.md)
- [OPlague — driver workbench motion contract](https://github.com/Ivan-VHS/OPlague/blob/main/docs/UI_MOTION.md)
- [VentuSample — Qt workstation motion contract](https://github.com/Ivan-VHS/VentuSample/blob/main/docs/UI_MOTION.md)
- [Pytyliki — processor workstation motion contract](https://github.com/Ivan-VHS/Pytyliki/blob/main/docs/UI_MOTION.md)

This constitution standardizes the contract and documentation. It does not mean the full 2×2 showcase, focus scene, OTA ring, AI diff stream, SVG/Canvas telemetry or a `--motion-scale` control is already implemented in every native application.
