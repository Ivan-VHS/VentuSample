# VentuSample plugin host

VentuSample now provides the same 18 internal products as built-in native plugins. A plugin is registered with a stable ID, display name, semantic version, area, declared capabilities and a handler. The Qt workstation exposes the registry from the Плагины menu.

## Shared plugin catalogue

| ID | Plugin | Main responsibility |
|---|---|---|
| operandu | Operandu | Typed generator parameter inspection |
| dropaul | Dropaul | Deterministic export dry-run planning |
| zdoro | Zdoro | Workspace and settings diagnostics |
| pyemont | Pyemont | Project and source inventory |
| dilly | Dilly | Interface and theme contract checks |
| baplap | Baplap | Exported WAV catalogue |
| sretuo | Sretuo | Test inventory and quality planning |
| kali | Kali | Bounded static security scan |
| zalam | Zalam | Manifest and export contract checks |
| liur | Liur | Renderer memory estimates |
| keel-koh | Keel Koh | Documentation index |
| vorof | Vorof | Static source tree reconstruction |
| vprof | Vprof | Static code-shape metrics |
| profe | Profe | Generator standards validation |
| prossee | Prossee | Export/release planning |
| vprossee | Vprossee | Requirements/evidence readiness |
| professiya | Professiya | Contributor learning paths |
| profevo | Profevo | Manifest and plugin evolution planning |

## Native plugin SDK

The public API is in src/plugin_manager.hpp. Implementations register a PluginDescriptor and a PluginHandler with PluginManager::registerPlugin. Handlers receive an immutable PluginContext containing a copy of the active generator parameters plus workspace and output paths. They return a PluginResult; the host catches plugin exceptions and blocks dispatch to disabled plugins.

Built-ins are compiled into ventusample_core. The current build does not load arbitrary DLL/SO files at runtime. This avoids implicit native-library loading while giving future first-party and reviewed extensions a stable in-process API.

## Desktop usage

Open Плагины → Менеджер плагинов to inspect capabilities, enable or disable modules, and run the selected tool against the active settings and selected export directory. Plugin state is stored through QSettings.

All built-in actions are bounded and read-only: they do not start generation, overwrite WAV files, execute shell commands, download code or alter project files. Dropaul returns a dry-run plan. Kali performs a static pattern scan and is not a complete security audit. Vprof reports source-shape estimates, not runtime profiling.

## Build and tests

```sh
cmake -S . -B build -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
