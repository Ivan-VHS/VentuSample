# VentuSample gaming audio workload profile

**Project author/developer:** Ivan-VHS

VentuSample now provides a bounded audio-buffer profile for game-oriented audio workloads. It accepts sample rate, channel count, target latency and an optional backend-selected buffer size, then reports the resulting buffer duration.

## Contract

Supported sample rates are 22,050–192,000 Hz, channel counts 1–8, target latency 5–200 ms and explicit backend buffers 64–4,096 frames. If a backend buffer is not supplied, the planner recommends a bounded power-of-two size and reports the actual buffer duration.

An optional exclusive-mode request is recorded only when the caller also supplies explicit consent. Authorization is not activation: this implementation does not open an output device, enable exclusive mode, change real-time scheduling priority or adjust OS thread affinity.

## Verification

```sh
cmake -S . -B build -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```


## Sequential rollout

Rollout gate: this PR stays in draft until DuckOS #75, OPlague #166, Pytyliki #213 and Reaper #253 are merged. Then it is the fifth stage of the gaming-policy rollout.

## DuckOS game identity

The request accepts the optional `gameId` used by `duckos.gaming-session-profile.v1`. When provided, the resulting audio profile preserves it so a native audio backend can correlate its buffer plan with the active game session. Leaving `gameId` empty remains valid for standalone audio workflows.
