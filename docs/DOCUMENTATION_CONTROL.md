# Documentation control

**Baseline date:** 2026-11-26 · **Project version:** 0.2.0 · **Scope:** generator, offline DSP, file formats, plugins, UI and integrations.

## Local publication gates

| Gate | Acceptance |
|---|---|
| Product identity | Exact product name, one-line scope, version and Apache-2.0 license. |
| Runtime | State clearly that rendering is offline; no audio-device callback or real-time playback. |
| Audio input | PCM16 mono/stereo WAV, declared rates, 128 MiB and 20M-frame limits. |
| Audio output | PCM16 WAV; export rates and metadata agree with code/tests/schema. |
| Resampling | Pitch changes and duration changes explained; do not call it time-stretch. |
| Determinism | Seed/variation and all nondeterministic inputs documented. |
| DSP | Parameter bounds, topology, trigger semantics, peak/RMS protection and buffers documented. |
| Plugin API | Registration, lifecycle and effect boundary are versioned and tested. |
| Quantum catalog | Separate module identities from DSP kernels and physical quantum computing claims. |
| Qt desktop | Optional target; missing Qt is a skipped build, not a successful desktop build. |
| Reaper integration | Manifest metadata consumer, not waveform render or audit. |
| Pytyliki adapter | Planned unless direct adapter and differential tests are present. |
| Rights | User responsibility for sample rights; no fabricated ownership/licensing claims. |
| Localization | EN/RU/UK/ZH-CN facts and section order aligned. |
| Navigation | Full doc catalog and working local links. |

## README review mapped to the 50-point standard

### Identity and first screen (1–10)
1. Exact product/repository name and technical purpose.
2. Metadata includes version, status, stack, platforms and license.
3. Record name/API/architecture transitions with evidence.
4. Place actual UI/architecture visual near the top with accurate caption.
5. State offline behavior and ownership boundaries immediately.
6. Quantitative DSP/quality claims require reproducible evidence.
7. List connected projects and explicit pack/ABI interfaces.
8. Add TOC for long or multi-subsystem README.
9. Keep chronology outside the opening purpose block.
10. Use precise engineering language.

### Chronology and releases (11–20)
11. Milestones must point to commits, tags or artifacts.
12. Disclose stack changes and rewrites only when evidenced.
13. Link exact release tags and dates.
14. Distinguish stable, experimental and draft distributions.
15. State pack v1/v2 compatibility and schema evolution.
16. State branch maintenance only when supported.
17. Keep release history separate from API docs.
18. Use version tables for pack and plugin matrices where useful.
19. Remove stale planned/closed work from active status.
20. Link major architecture steps to schemas and tests.

### Architecture and contracts (21–30)
21. Document offline-first, deterministic and bounded-processing principles.
22. Explain resampling, routing and plugin trade-offs.
23. Use short, precise paragraphs.
24. Balance text, tables, diagrams and examples.
25. Move detailed DSP contracts into docs/.
26. Separate schemas from implementation and roadmap.
27. Synchronize the four locale structures and terminology.
28. Distinguish modules, kernels, plugins, render tasks and assets.
29. Give each section a clear engineering question.
30. Support onboarding without hiding deep details.

### Breaking changes and limitations (31–40)
31. Disclose audio format, DSP and plugin API breaks.
32. Highlight data overwrite and sample-rights risks.
33. Explain reasons for manifest changes and migration.
34. Separate app limitations from OS/audio device/toolchain limits.
35. Compare DSP approaches through measured output, not adjectives.
36. State unsupported playback, plugin-hosting and GPU workflows.
37. Provide migration workarounds only when tested.
38. Retire old limitations after regression evidence.
39. Keep issue details in the tracker.
40. Clarify buffer ownership, thread use, input bounds and device/network access.

### Sources and maintenance (41–50)
41. Benchmark claims require host, build, sample and method.
42. Separate API, architecture, examples, RFC and standards.
43. Verify local and external links.
44. Identify versions of WAV/manifest/schema and DSP standards.
45. Cite audits with issuer/date/scope.
46. Link support, contribution and vulnerability reporting.
47. Link actual LICENSE, CONTRIBUTING and code-of-conduct files; state missing files.
48. Preserve external library and asset attribution.
49. Update docs with code, CLI, parameter and schema changes.
50. Test build and export commands from a clean environment.

## Evidence vocabulary

- **Specified:** contract exists; implementation is not claimed.
- **Planned:** accepted work not implemented.
- **Partial:** bounded subset works; omissions are explicit.
- **Implemented:** behavior exists in source and a real workflow.
- **Verified:** acceptance passed in a named environment and revision.

A module catalogue is not 133 independent DSP implementations. The 21 reusable kernels and 133 names must remain separately described. A manifest comparison is not waveform validation. Offline rendering is not real-time audio support.

## Before publication

- [ ] Verify CMake configuration with and without Qt.
- [ ] Run core and Qt-enabled tests on the appropriate hosts.
- [ ] Validate WAV and manifest schemas.
- [ ] Verify all changed paths and examples.
- [ ] Align all four root README locales.
- [ ] Preserve sample rights notices, Apache-2.0 and third-party licenses.
