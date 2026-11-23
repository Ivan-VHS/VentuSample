# Mora Amber native theme — VentuSample

**Maintainer:** Ivan-VHS

VentuSample now uses the shared Mora Amber profile by default. Light and Dark remain available through a named selector. Settings are stored under the existing Qt `QSettings` group; the legacy `dark` boolean is still written for older builds. `MORA_THEME_MODE=amber|mora|light|dark` can override the saved profile at launch.

## Canonical palette

| Role | Value |
|---|---|
| Workspace | `#211913` |
| Panel | `#33261D` |
| Primary text | `#FFF3E2` |
| Secondary text | `#E7D2B8` |
| Muted metadata | `#C4A98A` |
| Primary action | `#D58B3D` |
| Hover / focus | `#EAA653` / `#F3C16B` |
| Critical signal | `#B64A40` |
| Muted olive | `#526347` |
| Selection | `#60442E` |
| Structural edge | `#75543A` |

## Workstation boundary

The native palette is sourced from `mora_theme.hpp`. Parameter labels, units and control sizes remain explicit. Progress tracks use a linear amber fill; red is reserved for critical/destructive states. The visual profile does not change generator parameters, export formats or DSP processing.

Reduced Motion and Eco remain authoritative. GUI transitions stay on the GUI thread and never enter audio render workers or DSP callbacks.

## Verification

The Qt UI/UX test now checks the Mora Amber palette and text contrast alongside the previous Light/Dark profiles. Audio and generator output remain independently tested.
