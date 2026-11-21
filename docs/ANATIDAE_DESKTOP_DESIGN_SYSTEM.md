# Anatidae Desktop Design System — VentuSample

**Status:** Qt 6 light/dark theme and scrollable generator workbench are implemented. This document defines the product design authority; responsive reflow, keyboard/focus regression and physical HiDPI review remain release acceptance work until captured and tested.

## 1. Product intent

VentuSample is an offline procedural sample-pack workstation. The interface should feel like a precise musical instrument: clear phrase structure, responsive parameter adjustment, visible generation state and dependable export. The Anatidae identity adds calm waterline composition, feather-fine grouping and a recognizable blue/red product signature without turning the workstation into a cartoon or obscuring audio controls.

The application renders offline. The design must not imply live playback, real-time synthesis, device monitoring, network features or audio capture.

## 2. Anatidae lexical map

The supplied words are consolidated into design roles rather than exposed as feature labels:

| Design role | Source vocabulary | UI expression |
|---|---|---|
| First contact | Утёнок, Утёныш, Утеныш, Уточёнок, Утёночек, Утеночек, Утёнушка, Утюша, Утя, Утюня, Утюняш, Утюняшка, Утяшка, Кряквёнок, Крякушонок, Крякалыш, Птенчик, Птенец, Птеныш, Пухляш, Пушистик, Пернатик, Крылыш, Водоплавчик | Short first-run path, understandable defaults, grouped presets and useful empty states |
| Brand recognition | Утка, Уточка, Крякуша, Кряква, Кряквашка, Птица, Пташка, Пернатая, Крылатая | Abstract feather/wing mark, consistent header and navigation language |
| Surface and movement | Пуховичок, Пёрышко, Лапчатая, Прудовая, Озерная, Водоплавающая, Водяная птица | Fine separators, tactile controls, subtle waterline progress and phrase-oriented grouping |
| Experimental only | Крякозябра, Крякальщица | Internal inspiration; never production control names, warnings or status labels |

The source terms do not license baby-talk, quack puns or mandatory duck imagery. Technical copy remains direct and professional.

## 3. Semantic palette

Use the shared Anatidae white-blue-red system in both themes. Components must consume semantic roles rather than inventing local accent colors.

| Token | Light | Dark |
|---|---|---|
| Workspace | #F4F8FD | #071A2D |
| Surface / cards | #FFFFFF | #0F2A45 |
| Elevated / controls | #EAF1FA | #173A5E |
| Primary text | #122C45 | #E8EDF2 |
| Secondary text | #314B66 | #D0DCE8 |
| Muted text | #4D6177 | #B3C4D6 |
| Primary blue | #004E98 | #70A5FF |
| Blue hover / focus | #1A5FDB | #8AB6FF |
| Action / danger red | #B5122D | #FF7B86 |
| Red hover | #9C1026 | #FF9AA2 |
| Border | #CAD8E8 | #31516F |
| Selection | #D9E8FF | #244F82 |

Do not introduce teal, amber, green or violet as alternate brand accents. Status meaning is always conveyed by text and/or icon as well as color. Target WCAG AA for all actionable text and controls; aim for AAA for primary body text where the actual background permits.

## 4. Workbench composition

The existing generator stays the primary tab and remains scrollable. Preserve the established flow:

1. **Sound identity:** style, output type, key, scale, sampler, timbre and sample rate.
2. **Grid and structure:** BPM, bars, variations, seed, snare offset and bus compression.
3. **Groove and texture:** swing, density, grit, saturation and stereo.
4. **Master and motion:** filters, triplet rolls, micro-humanization and optional dust.
5. **Optional sample source:** explicit PCM16 WAV path, mix and pitch shift.
6. **Export station:** destination, create action, progress and status.

Use the same 8pt spacing rhythm (8/16/24/32 logical px), 8–12px corner radii and 44px preferred action targets. Keep field labels persistent and place units beside their values. Never rely on placeholder text alone. Cards are opaque, compact work surfaces; avoid low-contrast glass, decorative feather textures and oversized empty hero areas.

At narrow widths, stack the identity/structure and groove/master groups rather than compressing parameter fields. Keep the generator scrollable and retain access to Results/Log and About. Do not claim full responsive reflow until narrow, standard and wide window tests pass.

## 5. Brand geometry, iconography and motion

- Use a small abstract wing/feather motif or three calm waterline strokes only in brand chrome, section grouping or empty states. Avoid a cartoon duck mascot and avoid placing imagery behind controls.
- Group related parameter controls like feathers in a coherent fan: one label, one control, one value. The fan is a grouping metaphor, not a radial control layout.
- Treat the beat grid and phrase turnaround as the primary rhythmic geometry. Waterline progress may communicate generation state, but it must not fabricate percent completion where the worker only reports busy/finished.
- Use consistent 1.5–2px icon strokes, simple filled/outline states and recognizable file/audio/export symbols. Do not invent icons with ambiguous control meaning.
- Motion should clarify state transitions only. No autoplay, perpetual ripples or animated backgrounds. Honor reduced-motion preferences; keep progress and status text readable without animation.

## 6. Typography and accessibility

- Use the platform UI font (Segoe UI on Windows); reserve a readable monospace face such as Consolas for logs, paths and machine-readable output.
- Keep a visible keyboard focus indicator on buttons, tabs, editable fields, lists and checkboxes. Preserve logical tab order and standard Qt keyboard behavior.
- Maintain labels for every parameter; show units and current slider values as text.
- Disabled controls remain legible and clearly disabled. Errors and export outcomes must appear in a status surface, not only through color or log output.
- Do not shrink type to fit a wide panel. Long paths and logs remain selectable and scrollable.

## 7. Data, safety and behavior invariants

- Rendering remains offline on a worker thread; the UI stays responsive without claiming real-time callback guarantees.
- Source import remains bounded PCM16 mono/stereo WAV only. Imported sample paths and settings remain local.
- Export is PCM16 WAV plus a versioned manifest; existing output is not silently overwritten.
- Theme changes affect presentation only. They must not alter generator parameters, random seeds, output bytes, sample rate or the `ventusample.pack.v1` contract.
- Keep parameter validation and DSP in the existing C++ core; do not duplicate rendering or validation logic in widgets.

## 8. Implementation ledger

| Area | Baseline |
|---|---|
| Light/dark theme | Implemented in `MainWindow::applyTheme()`; semantic values in this document are authoritative |
| Preference persistence | Implemented through `QSettings` |
| Main generator | Scrollable Qt 6 workbench with grouped cards |
| Generation | Offline worker thread, progress/status and result/log tabs |
| Responsive reflow | Scroll fallback exists; full compact/standard/wide reflow is not certified |
| Focus regression | Must be checked across controls, tabs, lists and dialogs |
| HiDPI / 8K | Real-device visual regression remains required |

## 9. Release acceptance

- [ ] Light and dark themes use the exact semantic token family above.
- [ ] No unrelated accent palette or decorative water/feather texture is introduced.
- [ ] Keyboard focus is visible on every interactive control.
- [ ] Compact, standard and wide layouts keep all parameter labels, units and actions accessible.
- [ ] Theme changes do not mutate generation settings or pack output.
- [ ] Generation remains offline, bounded and responsive; no live-audio claim is made.
- [ ] PCM16 import/export and manifest compatibility tests pass.
- [ ] Real Windows/Linux Qt 6 captures are labelled by platform and are not replaced by mockups.
