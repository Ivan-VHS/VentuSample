# VentuSample plugin platform

VentuSample contains 18 first-party built-in plugins and a public C++20 SDK for reviewed, in-process handlers. The host also discovers local plugin.json manifests using the shared ivan-vhs.plugin.v1 contract.

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

## Manifest contract

The JSON Schema is schemas/ivan-vhs.plugin.v1.schema.json; a metadata-only fixture lives in examples/plugins/metadata-example/plugin.json. IDs are stable kebab-case, versions use SemVer, and api_version must equal 1. The native host currently supports a conservative plain-string subset of JSON manifests; escaped string values are not accepted by its lightweight metadata reader.

## Runtime and security boundary

Ventu handlers execute in-process and therefore share the workstation's OS privileges. The SDK is a reviewed registration interface, not a sandbox. Arbitrary DLL/SO loading, automatic executable discovery, network access and marketplace installation remain disabled. Manifest capabilities are descriptive metadata, not OS-enforced permission grants. Do not register unreviewed native code.

The plugin manager catches handler and lifecycle exceptions, records the last error and prevents a disabled or handler-less manifest from dispatching. A plugin failure is returned as a bounded PluginResult; it is not a guarantee that memory corruption or process-level faults can be contained in-process.

## Audio effect library

The **Audio Suite** is a second, DSP-oriented catalogue and does not overload the existing 18 workspace-tool handlers. Its stable descriptors and offline processor API are defined in src/audio_suite.hpp; the 66 registered processors are implemented in src/audio_suite.cpp. Each descriptor publishes a stable kebab-case ID, display name, category, description, UI concept and semantic version.

The Qt desktop opens the dedicated rack from **Плагины → Ventu Audio Suite — 66 DSP эффектов**. The user imports a bounded PCM16 mono/stereo WAV, adds effects in order, edits per-instance Amount/Wet/Rate/Tone/Width controls, and exports a new stereo PCM16 WAV. Processing runs on a worker thread and is transactional: a failed effect leaves the input buffer unchanged. The imported source is never overwritten.

The SDK is intentionally offline. It does not yet expose a real-time audio callback, VST3/AU binaries, arbitrary third-party code loading, or hardware capture. Complex concepts such as room reconstruction, neural timbre morphing and true phase-coherent spectral decomposition are represented by deterministic scalar DSP models, not claimed as full physical or ML implementations.

## Build and tests

```sh
cmake -S . -B build -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
