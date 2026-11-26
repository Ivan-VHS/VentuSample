# OCTA-8 portable values

VentuSample remains a C++20 renderer and offline DSP workstation. OCTA-8 provides the shared eight-kind vocabulary and C++ adapter in `src/octa8.hpp`. Its JSON wire contract is `ivan-vhs.octa8.v1`; the adapter does not enter the real-time audio path or replace the native renderer.

The eight kinds are null, boolean, signed 64-bit integer, finite number, UTF-8 text, bytes, sequence and record. Encoding remains a separate transport concern, so no JSON library or ABI dependency is forced on the audio core.

CLife remains the existing bounded, SHA-256-verified savepoint utility. Snapshots and recovery run offline and outside audio callbacks.
