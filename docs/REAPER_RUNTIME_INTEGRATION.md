# Optional Reaper native expression runtime

VentuSample remains a C++20 audio generator, DSP pipeline, plugin host and Qt workstation. The optional Reaper bridge supplies a deterministic integer-expression runtime for future control and preset logic; it does not replace the audio engine or run on the real-time DSP path.

Enable it by pointing CMake at an explicit local Reaper checkout:

```sh
cmake -S . -B build -DVENTU_BUILD_DESKTOP=OFF -DREAPER_SOURCE_DIR=/absolute/path/to/Reaper
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The `ventusample_core` target links Reaper's versioned C ABI v1 when enabled. The small C++ wrapper in `src/reaper_expression.hpp` exposes:

- `reaperRuntimeAvailable()` — reports whether the ABI was linked.
- `evaluateReaperInteger(expression, result, error)` — evaluates bounded signed-i64 expressions.
- `lookupReaperSelfhood(identity, ordinal, error)` — exact lookup in the canonical 20-entry vocabulary.

When the option is omitted, the project builds exactly as before and the wrapper returns an explicit unavailable diagnostic. No runtime shared-library search, PATH lookup, download, filesystem access or process execution is introduced by this bridge.

The Reaper evaluator accepts only the documented native integer-expression subset. Overflow and division by zero fail closed. It is not a general Reaper-language runtime or a sandbox.

VentuSample retains ownership of waveform loading, rendering, sample placement, stochastic routing, plugin lifecycle, audio buffers, Qt and export. The Reaper bridge is optional control-language infrastructure, not a replacement for specialist C++ audio code.
