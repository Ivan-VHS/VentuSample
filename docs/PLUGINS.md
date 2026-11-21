# VentuSample plugin platform

VentuSample contains 19 first-party built-in plugins and a public C++20 SDK for reviewed, in-process handlers. The host also discovers local plugin.json manifests using the shared ivan-vhs.plugin.v1 contract.

## Host capabilities

| Capability | Implementation |
|---|---|
| Built-in registry | 18 first-party plugins registered by the core |
| Public SDK | PluginDescriptor, PluginContext, PluginHandler and lifecycle callbacks |
| Manifest discovery | metadata-only recursive discovery from the desktop manager |
| Compatibility | plugin API version 1, host ID ventu and semantic plugin versions |
| Lifecycle | lazy initialization before first dispatch; shutdown on disable |
| Handler attachment | reviewed C++ handler registration through registerPlugin or attachHandler |
| Diagnostics | source, manifest path, enabled/initialized state, handler attachment and last error |
| Safety | discovered metadata defaults disabled; no automatic DLL/SO loading or manifest-driven code execution |

## Desktop usage

Open **Плагины → Менеджер плагинов**. Use **Обнаружить расширения…** to scan a local directory for plugin.json files. Discovery adds metadata to the list only; it does not execute the manifest entrypoint, load a shared library or grant capabilities. The details pane shows the manifest source, API level and whether a reviewed handler is attached.

An extension that has passed code review is connected by a host-side registration function using the public SDK:

```cpp
ventu::PluginDescriptor descriptor{
    "studio-tool", "Studio Tool", "Workflow", "1.0.0",
    "Reviewed in-process extension.", "inspect", {"project.read"}
};
manager.registerPlugin(
    descriptor,
    [](const ventu::PluginContext& context) {
        return ventu::PluginResult{true, "Inspection complete", {}};
    },
    false,
    [](const ventu::PluginContext&) { /* initialize resources */ },
    [](const ventu::PluginContext&) { /* release resources */ }
);
```

A discovered manifest can receive a reviewed handler with attachHandler(id, handler, onInitialize, onShutdown). A metadata-only entry cannot be enabled or executed until a handler is attached. Initialization is lazy and occurs before the first successful dispatch. Disabling an initialized module invokes its shutdown callback.

## Quantum Audio Engine — 133 DSP modules

The audio laboratory is opened independently from **Плагины → Quantum Audio Engine — 133 DSP**. It has 133 stable DSP module identities in five families: Quarks & Hadron (23), Quantum Mechanics (23), Atoms & Orbitals (23), Fields & Relativity (23), and Experimental Instruments (41).

The lab loads supported WAV files, offers Amount / Tone / Rate / Wet-Dry controls for each module, builds an ordered processing chain, processes interleaved float samples offline and exports stereo PCM16 WAV. Module settings and chain order are persisted locally with QSettings. The catalogue uses 21 reusable DSP kernels with controlled per-module variants, rather than claiming 133 unrelated low-level algorithms. Each entry has a stable ID, description, category, visual concept and tested dispatch path.

The audio processor is deliberately offline: it does not introduce a real-time callback, device routing, VST/AU hosting or arbitrary native-library loading. Quantum and particle terminology describes creative signal-processing models, not physical quantum computation. The complete catalogue, DSP mapping and audio limitations are in [QUANTUM_AUDIO_ENGINE.md](QUANTUM_AUDIO_ENGINE.md).

## Stochastic Router

The **Stochastic Router** is a first-party offline DSP processor. It does not load or randomly instantiate arbitrary third-party DLLs. Its weighted pool contains eight processors: low-pass, high-pass, saturator, bitcrusher, delay, reverb, ring modulation and stereo width. Each entry can be enabled or excluded from the generator UI.

- **Topologies:** hybrid, serial or parallel (2–4 branches when enough effects are selected).
- **Triggers:** one route per file, one route per bar, or a route per scheduled generator event. Onset mode uses event frames from the procedural renderer; imported WAVs are not independently transient-detected.
- **Macros:** intensity 0–100%; aggression, space, dirt and chaos follow grit, stereo, saturation and density controls.
- **Safety:** wet ≤65%, effect feedback ≤72%, 5–20 ms dry/wet fades at segment boundaries, RMS compensation limited to 0.5–2×, sample-peak output guard ≤−0.3 dBFS.
- **Determinism:** generator seed, variation, topology, trigger and enabled pool mask determine route selection.

The generator checkbox is off by default. When enabled, the engine processes the rendered float buffer before PCM16 WAV encoding. Manifest v2 stores the settings and each output file's selected route names. Running the module from Plugin Manager performs a one-bar in-memory preview; it does not write files.

This path is **offline**. It does not provide a real-time callback, playback hot-swapping, VST/AU hosting or arbitrary third-party audio-plugin loading.

## Manifest contract

The JSON Schema is schemas/ivan-vhs.plugin.v1.schema.json; a metadata-only fixture lives in examples/plugins/metadata-example/plugin.json. IDs are stable kebab-case, versions use SemVer, and api_version must equal 1. The native host currently supports a conservative plain-string subset of JSON manifests; escaped string values are not accepted by its lightweight metadata reader.

## Runtime and security boundary

Ventu handlers execute in-process and therefore share the workstation's OS privileges. The SDK is a reviewed registration interface, not a sandbox. Arbitrary DLL/SO loading, automatic executable discovery, network access and marketplace installation remain disabled. Manifest capabilities are descriptive metadata, not OS-enforced permission grants. Do not register unreviewed native code.

The plugin manager catches handler and lifecycle exceptions, records the last error and prevents a disabled or handler-less manifest from dispatching. A plugin failure is returned as a bounded PluginResult; it is not a guarantee that memory corruption or process-level faults can be contained in-process.

## Build and tests

```sh
cmake -S . -B build -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
