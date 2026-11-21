# CLife Immortality Pod — VentuSample C++20 adapter

VentuSample maps the simulation to deterministic offline project/render state. A snapshot
copies a caller-provided byte vector, binds it to a project and generation, and verifies
the payload before export. Recovery is advisory and never runs inside the real-time audio
path, plugin dispatch, renderer callback or Qt UI thread.

Implemented API: capture, verify, restore-to-caller-buffer, typed fault handling and four
recovery branches. The payload limit is 64 MiB. Recovered bytes are not automatically
loaded as plugin code or executed as a project.

The portable implementation uses FNV-1a 64-bit solely for accidental-corruption detection.
It is not cryptographic and does not authenticate a checkpoint. Durable storage, retention
and access control remain responsibilities of the host application. Use signed provenance
if tamper resistance is required.

Build and test:
cmake -S . -B build -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
