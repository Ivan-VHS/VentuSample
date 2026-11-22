# Contributing to VentuSample

VentuSample is a local, offline sample-generation workstation. Preserve deterministic rendering, bounded output sizes, honest audio metadata and the separation between the DSP core and the optional Qt desktop shell.

## Before opening a pull request

1. Add a deterministic core test for DSP, validation or export behaviour.
2. Keep expensive rendering off the Qt event thread and join worker threads safely.
3. Validate all user-controlled settings before rendering or writing files.
4. Do not embed local source-sample paths in portable presets unless the user explicitly exports that path.
5. Do not claim copyright, ownership or licence clearance for generated or imported audio.

## Local verification

```sh
cmake -S . -B build -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

The DSP core and its tests should remain buildable when Qt 6 is unavailable. When changing the Qt shell, test the desktop target on a Qt-enabled host.

## Pull request expectations

Describe changes to sound, parameter ranges, file formats and reproducibility. Include a sample manifest or screenshot when it improves review, but do not commit large rendered audio packs. Avoid issue-closing keywords in PR descriptions; maintainers close issues after review.

## Shared report format

To wrap an existing JSON result in the common report metadata contract, run python tools/report_envelope.py --tool ventusample --version VERSION < result.json. Keep the tool-specific payload unchanged. Tests live in tests/test_report_envelope.py.
