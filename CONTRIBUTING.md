# Contributing to VentuSample

VentuSample is an offline sample-generation workstation. Preserve deterministic rendering, bounded output sizes, honest audio metadata and the separation between the DSP core and the optional Qt desktop shell.

## Before opening a pull request

1. Add a deterministic native test for generator, DSP, validation or export behavior.
2. Keep expensive rendering off the Qt event thread and join worker threads safely.
3. Validate user-controlled settings, buffer sizes, sample rates and file bounds before processing.
4. Do not export machine-specific source-sample paths in portable presets unless explicitly requested by the user.
5. Do not claim copyright, ownership or licence clearance for generated or imported audio.
6. Keep plugin discovery metadata-only unless a reviewed native handler is registered.
7. Update all affected English, Ukrainian, Russian and Simplified Chinese documentation.

## Local verification

~~~sh
cmake -S . -B build -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
git diff --check
~~~

For a portable build without Qt:

~~~sh
cmake -S . -B build-core -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

## Pull request expectations

Describe the audio path, sample format, numerical limits, seed behavior, thread lifecycle and compatibility impact. Attach only genuine application screenshots; label workflow diagrams as illustrations. Keep claims aligned with [FEATURES.md](docs/FEATURES.md) and [ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Security and rights

Do not commit credentials, private local paths or unlicensed source material. A hash identifies bytes but does not establish ownership or licence clearance. See [SECURITY.md](SECURITY.md).
