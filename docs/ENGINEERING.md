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
