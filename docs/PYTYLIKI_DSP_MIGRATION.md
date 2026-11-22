# Migrated C11 processing modules

VentuSample is the sole owner of the migrated audio-processing code. The portable C11 modules formerly maintained inside Pytyliki now live under `src/compat/pytyliki-dsp/` and are built as `ventusample_pytyliki_dsp`.

## Transferred source

| File | Responsibility |
|---|---|
| `dsp.c` / `dsp.h` | bounded PCM16 WAV import/export, tone generation, gain, low-pass, compression, saturation and measurements |
| `dsp_engine.c` / `dsp_engine.h` | scalar block processing, biquad and FIR filters, limiter and block statistics |

The original boundary and processing-engine tests moved with the implementation to `tests/compat/`. They are compiled and registered with CTest; they are not left as unbuilt reference copies.

## Build contract

The project now enables both C11 and C++20. The imported module is compiled as C11 and linked into `ventusample_core`; it remains a separately addressable static target for callers that want the original API. On Unix-like systems the target links the platform math library.

```sh
cmake -S . -B build -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The module remains offline and buffer-based. It does not add an audio-device host, real-time callback, plugin loader or network service.

## Manifest audit

Pack-v1 validation and normalized metadata comparison are maintained in `tools/ventusample_pack_audit.py`. The JSON Schema is in `schemas/ventusample.pack.v1.schema.json`; test coverage is in `tests/test_ventusample_pack_audit.py`. The audit reads manifest JSON only and does not open or modify files listed by the manifest.

A standalone example is available at `examples/compare_packs.py`.
