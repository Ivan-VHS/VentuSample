# Engineering and release gates

| # | Gate | Acceptance |
|---:|---|---|
| 1 | Determinism | fixed seed + parameters → repeatable output |
| 2 | Bounds | validate BPM, bars, variations, WAV bytes and frames |
| 3 | Audio | finite samples, PCM format and sample-peak ceiling |
| 4 | Export | preserve existing files; validate manifest and relative paths |
| 5 | Tonality | tag pitched files; never invent a drum key |
| 6 | Threading | UI responsive; renderer has no audio callback |
| 7 | Rights | operator confirms source permission |
| 8 | Compatibility | validate v1/v2 schemas and legacy reads |
| 9 | Release | build, tests, schemas, license and clean package |

VentuSample is an offline renderer, not a DAW, plugin host, live monitor or sample-rights clearance service.

## Shared evolution matrix

This repository adopts the shared [Engineering Evolution Framework](ENGINEERING_EVOLUTION_FRAMEWORK.md): 20 domains, 40 practices, a versioned JSON contract and negative tests. The framework complements existing engineering, project and safety contracts; it does not assert that every technique is applicable or already implemented. Claims of implementation must remain backed by local code, tests or operational evidence.
