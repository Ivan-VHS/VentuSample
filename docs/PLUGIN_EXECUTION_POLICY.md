# Plugin execution policy and operational health

VentuSample is a deterministic, offline C++20/Qt audio workstation. The Java
framework comparison is used to strengthen its native plugin-host contracts;
no Spring, Quarkus or servlet runtime is introduced into the renderer.

## Per-request execution policy

Each `PluginContext` now carries a `PluginExecutionPolicy`:

| Parameter | Default | Meaning |
|---|---:|---|
| `maxEstimatedOutputBytes` | 512 MiB | Request-level preflight output budget |
| `softDeadlineMs` | 120,000 ms | Observed execution budget |
| `requireDeterministicSeed` | true | Declares deterministic-generation intent |

The host validates `Parameters` before dispatch and estimates the audio-pack
size using the generator's duration, sample-rate, channel/PCM size and variation
count. A request that exceeds its own configured output budget is rejected
before the handler runs.

Each plugin status exposes invocation, success/failure and late-deadline
counters, last/max duration, last execution ID and last status. The
`PluginManager::health()` snapshot reports registry readiness and handler
attachment.

## Deadline and concurrency semantics

The plugin manager calls synchronous handlers. Its deadline is observational:
a handler that returns late is marked `deadlineExceeded` and receives an
explicit diagnostic. C++ code is not forcibly terminated while rendering.
The manager currently dispatches handlers synchronously; it does not claim a
thread-pool, parallel renderer or per-plugin concurrency scheduler.

A future concurrent host must add explicit admission limits, cancellation
points in the DSP pipeline, cancellation-safe output staging, thread-safe
lifecycle transitions and tests for shutdown while work is in flight.

## Java framework capability crosswalk

| Java capability | VentuSample adaptation |
|---|---|
| Spring DI / lifecycle hooks | Existing explicit PluginContext plus initialize/shutdown callbacks |
| Quarkus build-time validation | CMake/compiler validation and request preflight before render |
| Micronaut compile-time metadata | Versioned plugin manifest and API version checks; no runtime reflection |
| MicroProfile Health / Actuator | PluginHealth and per-plugin invocation metrics |
| Resilience4j bulkhead | Not yet implemented; current plugin invocation is synchronous |
| Resilience4j timeout | Observed soft deadline, not forced cancellation |
| Hibernate / jOOQ | Not applicable to offline generation; presets and packs are file-based contracts |
| Vert.x / Netty | Not applicable to the current offline renderer; no event-loop networking |
| Spark / Flink / Kafka Streams | Not applicable; deterministic local render batches are not distributed stream processing |
| Camel EIP | Explicit manifest exchange and opt-in Reaper/Pytyliki interfaces, not a general integration bus |

## Verification

```sh
cmake -S . -B build -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The adaptation branch adds preflight-budget, invocation-metric and observed
deadline regression checks. Run the CMake test suite before merging.
