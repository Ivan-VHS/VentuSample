# VentuSample UI/UX

| Surface | Contract |
|---|---|
| Generator | primary task; sound identity → grid → groove → master → source → export |
| Results/Log | separate tabs; readable, selectable and bounded |
| Create action | visible destination, progress, no duplicate render |
| Theme | light/dark tokens only; no musical setting changes |
| Renderer | worker thread; UI stays responsive |
| Source | explicit local PCM16 WAV; no capture |

Platform target: Qt 6 Widgets. Preserve keyboard order, focus, readable disabled state and explicit units. Responsive reflow must be tested before claimed. See [design tokens](ANATIDAE_DESKTOP_DESIGN_SYSTEM.md).

## 40026 Codex

The normative six-cycle / 666-rule contract is maintained in [UI/UX Codex 40026](UI_UX_CODEX_40026.md). Qt now provides a tested contrast policy, accessible names for buttons/fields/choices, label-to-slider associations, and larger interactive targets. Worker and DSP execution remain separate from all accessibility and presentation code. Manual keyboard-only, high-DPI and 200% scaling checks remain part of release validation.

