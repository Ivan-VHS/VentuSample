# Mora integration

This directory describes the existing VentuSample desktop executable as an optional application for a future native Mora launcher.

## Manifest

\`manifest.json\` follows the DuckOS Mora application manifest v1.0.0 contract. It records the platform-specific Qt desktop entry points:

- Windows: \`build/Release/VentuSample.exe\`
- Linux: \`build/VentuSample\`

Both targets require the desktop build to have completed with Qt 6 Widgets available. The portable core build does not produce a desktop application.

The manifest is **specified, not activated**. The current Mora HTML shell does not launch native programs; no process broker, packaging hook or device/audio permission is introduced here.

## Build

Use the project's current CMake workflow. For Windows, configure with MSVC and Qt 6 Widgets; for Linux, use the documented CMake/Ninja/Qt 6 desktop build. Run CTest before using the generated executable.

## Security and audio boundary

A native launcher must validate the manifest and executable path under a trusted installation root, choose only the entry point for the current platform, pass arguments without shell interpolation and request explicit operator approval. VentuSample's audio generation and DSP remain offline; this manifest does not turn it into a DAW, real-time audio host or audio-device service.

Project: [Ivan-VHS/VentuSample](https://github.com/Ivan-VHS/VentuSample). Project author/developer: Ivan-VHS.