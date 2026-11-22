# Reaper language integration

VentuSample remains the native C++20 audio generator, DSP pipeline and workstation. Reaper 0.2 is used as an optional offline orchestration and metadata-audit layer; it does not enter the real-time audio callback, render audio, decode WAV files or replace the C++ DSP engine.

## Executable example

\`automation/reaper/pack_audit.rpr\` consumes a generated \`ventusample.pack.v2\` JSON manifest. It validates tonal and stochastic metadata, derives normalized metrics, performs a read-only self-comparison, and emits a versioned Reaper comparison report.

The integration CI first runs the existing native generator test, which creates a real fixture under the system temporary directory. \`automation/reaper/prepare_fixture.py\` checks the exact v2 schema and copies the manifest into the bounded CI workspace. Reaper then parses and executes the script with both its tree-walk and bytecode VM engines.

## Run from the repository root

\`\`\`sh
cmake -S . -B build -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel --target ventusample_tests
ctest --test-dir build --output-on-failure -R '^ventusample-generator$'
python automation/reaper/prepare_fixture.py
PYTHONPATH=/path/to/Reaper python -m reaper --check automation/reaper/pack_audit.rpr
PYTHONPATH=/path/to/Reaper python -m reaper --engine vm automation/reaper/pack_audit.rpr
\`\`\`

## Boundaries

- VentuSample owns audio generation, waveform processing, sample placement, stochastic routing, plugin lifecycle and export.
- Reaper owns only offline orchestration, JSON contract validation and report generation.
- WAV files are not opened by Reaper's manifest adapter.
- No native library is loaded by this script; the separate legacy C ABI expression bridge is not needed for this workflow.
- The Reaper Python runtime is the language reference. Full C17 Reaper compiler parity is not claimed.
