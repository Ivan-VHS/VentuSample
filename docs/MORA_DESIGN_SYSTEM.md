# Mora integration · VentuSample

VentuSample's Qt desktop stylesheet now consumes the shared Mora palette through `src/mora_theme.hpp`. Page/surface colors, the purple primary action, teal keyboard focus, selection, and semantic danger colors are centralized in a small constexpr-friendly adapter.

The existing Qt UI, preset workflow, DSP controls, generation behavior, plugin-manager boundaries, BPM indicator and motion preferences remain in place. The stylesheet now uses the shared Mora corner/spacing feel while preserving the studio's dense parameter layout.

The portable token mirror is `mora/theme.json`; the native UI remains a Qt application rather than an embedded browser.
