# Cross-repository plugin federation

VentuSample participates in the shared 18-plugin catalog while keeping its own C++20 native host. The canonical catalog and this host's capability declaration live in \`federation/plugins/\`.

## Use the native host

Open **Плагины → Менеджер плагинов** to inspect and dispatch built-in tools. Native handlers are registered through \`ventu::PluginManager\` and the reviewed C++ SDK. The cross-repository catalog is metadata; it does not cause arbitrary foreign code to be loaded.

Local \`plugin.json\` discovery remains metadata-only. A discovered entry stays disabled until a reviewed handler is attached through the host SDK. The manager does not load arbitrary DLL/SO files or execute manifest entrypoints.

## Federation rules

- \`catalog.v1.json\` is the same versioned offline registry used by Reaper, OPlague and Pytyliki.
- \`host.v1.json\` identifies VentuSample as the C++20 execution host.
- The 18 shared identities map to local reviewed native operations. Audio generation and DSP remain owned by VentuSample's C++20 core.
- Cross-repository dispatch requires a separately implemented adapter; no network discovery or implicit remote execution exists.
- Capabilities are descriptive labels, not sandbox permissions. In-process handlers share the application's OS privileges.

## Compatibility

Catalog schema: \`schemas/ivan-vhs.plugin-catalog.v1.schema.json\`.
Native manifest schema: \`schemas/ivan-vhs.plugin.v1.schema.json\`.
Host contract: \`ivan-vhs.plugin-host.v1\`, API version 1.
