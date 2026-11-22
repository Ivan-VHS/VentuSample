# Native C11 processing modules

The native C11 modules in `src/native_dsp/` are owned and built by VentuSample. They are offline processing components; the desktop renderer and Quantum Audio Engine remain the primary C++20 application path.

| Module | Responsibility |
|---|---|
| `wav_processor.c` | bounded PCM16 WAV load/write, tone generation, gain, low-pass, compression, saturation and signal measurements |
| `block_engine.c` | bounded scalar biquad, FIR and limiter processing plus block-level measurements |

## Public API

- `ventu_audio.h` exposes the owned audio buffer, WAV I/O, processing and statistics functions.
- `ventu_block_engine.h` exposes the bounded block engine, reusable filter state and limiter.

Both headers provide C linkage when included from C++. The APIs use the `ventu_*` namespace and do not depend on Pytyliki headers or runtime code.

## Build and tests

The project builds C11 and C++20 sources into `ventusample_core`. The migrated WAV container-boundary checks and block-engine regression tests are registered with CTest:

```sh
cmake -S . -B build -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The processor validates input lengths, sample rates, channels, finite values and bounded allocations. These kernels are not an audio-device driver, real-time callback, VST/AU host or a replacement for the existing Quantum Audio Engine.
