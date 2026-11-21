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
