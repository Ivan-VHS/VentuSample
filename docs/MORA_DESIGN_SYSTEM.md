# Mora integration · VentuSample

VentuSample's Qt desktop stylesheet consumes the shared Mora palette through `src/mora_theme.hpp`. Page/surface colors, the purple primary action, teal keyboard focus, selection and semantic danger colors are centralized in a small native adapter.

The existing Qt UI, preset workflow, DSP controls, generation behavior, plugin-manager boundaries, BPM indicator and motion preferences remain in place. The stylesheet uses the shared Mora corner and spacing language while preserving the studio's dense parameter layout.

The portable token mirror is `mora/theme.json`; the native UI remains a Qt application rather than an embedded browser.

## Implementation map

| Surface | Mora behavior |
|---|---|
| Page / panels | shared indigo and light surfaces |
| Primary controls | violet action and matching foreground |
| Focus | teal keyboard-focus accent |
| Selection | shared semantic selection fill |
| Danger states | contrast-safe semantic danger pair |
| Dense studio controls | Qt-native widgets and existing layout |

The Qt target includes `mora_theme.hpp` directly. Rendering, WAV I/O, DSP, presets and plugin lifecycle continue to run through their existing native code paths.
