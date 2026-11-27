# Ecosystem Cloud Adapter — VentuSample

**Adapter:** `ventusample` · **Shared contract:** `ivan-vhs.ecosystem-cloud.v1` · **Status:** contract-only

VentuSample remains an offline-first audio engine and native desktop workbench. Cloud services do not participate in DSP, rendering, project loading or plugin lifecycle.

## Eligible data

The future adapter may publish pack and preset manifests, user-published preset packs and release-artifact metadata. A publish action must show the exact files and metadata before upload.

## Local-only by default

Audio recordings, source sessions, raw waveform captures, render caches, private plugin state and plugin binaries are never uploaded automatically. Offline rendering and playback remain available without account credentials or network access.

## Runtime rules

- Cloud access is disabled by default.
- No account prompt or network call occurs during audio engine startup or render.
- Uploads use the shared private bucket and are capped at 100 MiB per object.
- No public object URLs are created; downloads require membership and explicit user action.
- Retries are bounded and idempotent. Pack conflicts are handled by versioned pack semantics, not blind overwrite.

## Implementation boundary

This PR contains the adapter manifest, data policy and regression test only. It adds no SDK, HTTP dependency, remote audio rendering, plugin download path or change to the C++20/Qt runtime. Transport and artifact publishing require a separate native integration PR.
