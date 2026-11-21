# Elbrus E2K support

## Support status

VentuSample separates the offline C++20 DSP/core engine from the optional Qt desktop shell.

| Layer | E2K status |
|---|---|
| Offline DSP / deterministic generator core | Candidate native build with MCST LCC; requires native CTest validation |
| WAV and JSON output | Portable file formats; validate byte-for-byte and metadata tests on target |
| C++ language/library support | LCC 1.31 reports partial (`±`) C++20 support; successful complete project build is not assumed |
| Qt 6 desktop UI | Optional and dependent on an E2K-compatible Qt 6 Widgets package; disabled in baseline E2K CI |
| Plugin manifest inspection | Metadata-only and independent of loading native plugin code |
| Native plugin handlers | Must be compiled for the E2K ABI; do not load x86-64 DLL/SO binaries |

MCST's published LCC language matrix lists C++17 as supported in LCC 1.31 and C++20 as partial.
The project currently requires C++20. Do not silently lower the project standard to make a build pass:
record any unsupported feature as a compatibility issue and fix it with a reviewed, portable
implementation or an explicitly approved standard change.

## Toolchain and build

Use an MCST LCC package matching the exact Elbrus CPU generation and Elbrus Linux release.
MCST documents native `g++` as an LCC C++ driver alias; for a cross build, use the executable
provided by the licensed cross-toolchain package.

- [MCST SDK / compiler versions](https://www.mcst.ru/SDK)
- [LCC language support matrix](https://www.mcst.ru/LCC)
- [MCST developer downloads](https://dev.mcst.ru/download/)
- [Elbrus Linux CPU-generation matrix](https://www.mcst.ru/elbrus_linux)

On a native E2K host:

```sh
CXX=g++ bash scripts/elbrus/build-e2k.sh
```

The baseline configures `VENTU_BUILD_DESKTOP=OFF`, builds the C++20 core and test executables,
and runs CTest on the host. Qt desktop packaging is not claimed by this baseline.

For cross compilation, use `cmake/toolchains/elbrus-e2k.cmake` with explicit `E2K_CXX` and,
when required, `E2K_SYSROOT`:

```sh
E2K_CXX=/opt/mcst/bin/e2k-g++ E2K_SYSROOT=/opt/mcst/sysroot \
  cmake -S . -B build-e2k-cross \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/elbrus-e2k.cmake \
  -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-e2k-cross --parallel
```

A cross-compiled executable must be run on matching hardware or a separately validated
`qemu-e2k` setup. The link step alone does not prove correct C++ runtime behavior.

## Performance and deterministic output

Keep the scalar renderer as the reference path. Benchmark on target hardware using fixed seeds,
identical parameters and repeated runs. Compare output hashes and manifest values as well as timing.
Do not add E2K-specific hand scheduling or assembly until profiling demonstrates a bottleneck and
the fallback path has equivalent tests.

## CI

`.github/workflows/elbrus-e2k.yml` is manually dispatched on a self-hosted runner labelled
`self-hosted, linux, e2k`. It validates the offline core without requiring Qt; the normal Linux
and Windows workflows remain unchanged.
