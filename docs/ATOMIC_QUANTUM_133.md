# Atomic-Quantum 133 — VentuSample research layer

VentuSample keeps its existing **Quantum Audio Engine — 133 DSP modules** separate from this atomic/quantum function catalogue. The two lists have different IDs and contracts: audio DSP transforms sample buffers, while this catalogue describes bounded classical simulations of quantum states, measurements, noise, atomic lattices and algorithm telemetry.

## Families

| Ordinals | Family | Scope |
|---|---|---|
| 001–022 | State | Qubit/atom initialization, normalization, rotations and level models |
| 023–044 | Entanglement | Bell/GHZ/W construction and correlation measurements |
| 045–066 | Measurement | Projective sampling, readout, feedback and telemetry |
| 067–088 | Noise / ECC | Channels, syndrome primitives and code helpers |
| 089–110 | Atomic dynamics | Lattice, potential and transport models |
| 111–133 | Algorithms / telemetry | Algorithmic kernels and health metrics |

## Separation and limits

- Canonical manifest: `schemas/atomic-quantum-functions.v1.json`; schema: `schemas/atomic-quantum-functions.v1.schema.json`.
- Atomic/quantum state simulation is a classical numerical research layer, not quantum hardware execution.
- Audio DSP remains in `quantum_audio.*`; the atomic catalogue must not silently reinterpret, rename or replace those 133 DSP modules.
- Seeded simulation and finite-value validation are required for reproducible results. Unsupported research operations remain catalogued rather than being presented as validated physical models.

See [Quantum Audio Engine](QUANTUM_AUDIO_ENGINE.md) for the independent audio DSP catalogue.
