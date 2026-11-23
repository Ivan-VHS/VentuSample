# VentuSample DuckOS palette

VentuSample uses the same fixed palette as DuckOS and Mora.

| Role | Value |
|---|---|
| Page | `#080808` |
| Surface | `#151515` |
| Main text | `#FFFFFF` |
| Supporting text | `#E5E5E5` |
| Muted text | `#A6A6A6` |
| Accent / action | `#FFD400` |
| Hover / focus | `#FFE45C` |
| Border | `#3A3A3A` |
| Selection | `#3A3100` |

The desktop has one palette. Legacy `MORA_THEME_MODE` and saved dark-mode preferences no longer select another appearance. Other generator, DSP, preset and plugin preferences remain untouched.

The Qt style keeps at least 4.5:1 contrast for ordinary text and interactive controls, and a 3:1 focus indication. Beat synchronization, DSP meters and errors retain explicit text labels and accessible names; yellow is shared action color, not the only state signal.

The drapery banner uses the same tokens. Its timer stays on the UI thread, stops under Reduce Motion, Eco Visuals or `UI_REDUCED_MOTION`, and does not touch generator or DSP callback execution.
