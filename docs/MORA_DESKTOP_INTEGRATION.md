# Mora desktop integration — VentuSample

VentuSample joins Mora as the native procedural audio workstation. Mora provides discovery, the common desktop surface and the startup theme. The renderer, offline DSP, preset storage and export path remain in VentuSample.

## Workspace contract

- Manifest: mora-app.json, version 1.
- Runtime: native C++20 application built by the existing CMake target VentuSample.
- Desktop toolkit: Qt 6 Widgets.
- Theme hand-off: MORA_THEME_MODE=dark or light, read after the user's local settings and before the first theme is applied.
- Window model: a separate OS-managed native window.

Mora discovers the existing Qt build output. It does not claim that the application is ready when Qt 6 is missing or the desktop target has not been built. Build and platform requirements remain governed by CMakeLists.txt.

The shared contract and workspace configuration live in [DuckOS / Mora](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/MORA_DESKTOP.md).

## Visual convergence

The application keeps its current Qt 6 widget hierarchy and accessibility defaults. Only the initial light/dark choice is inherited from Mora. The palette continues to use the same blue primary action, restrained red signal role, opaque work areas and visible keyboard focus.

The environment value is a startup preference, not remote live control. A theme chosen later inside VentuSample is still local to the application session and saved using its existing settings mechanism.

## Product boundaries

- Audio generation and WAV processing remain offline operations.
- Mora does not become a DAW, plugin host, audio-device router or playback service.
- Audio buffers, render workers and DSP code are not coupled to desktop animation.
- Presets and generated output remain local files.
- The optional Reaper bridge and Pytyliki DSP compatibility code retain their current build boundaries.

## Verification

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
    cmake --build build --parallel
    ctest --test-dir build --output-on-failure

Qt 6 Widgets must be available for the native desktop target. The shared Mora integration does not change the portable DSP/core build.
