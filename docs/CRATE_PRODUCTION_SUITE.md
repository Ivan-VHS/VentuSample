# CRATE Production Suite
## Hip-Hop beatmaking, crate work, groove design, sample craft and record-ready prints

**CRATE** means **Crate, Rhythm, Arrangement, Texture and Export**. It is VentuSample's hip-hop production architecture: a connected studio workflow for selecting source material, chopping samples, locking the pocket, building drums and loops, shaping texture, balancing the beat and printing a reproducible WAV pack.

CRATE replaces the former abstract autonomous-development framing with an entirely music-production vocabulary. It does not turn VentuSample into a code-mutating agent, a live DAW, a recording service or a third-party plug-in host. Every room in CRATE serves a beatmaking task.

## 1. The CRATE sound

CRATE is built around hip-hop production practices:

- crate digging and source selection;
- sample chopping, pitching and phrase rebuilding;
- boom-bap, East Coast, Golden Era, G-Unit and other documented beat profiles;
- kick, snare, hat and percussion voicing;
- swing, pocket, snare placement and triplet movement;
- bass, keys, melody and loop construction;
- vinyl colour, saturation, filtering, delay, reverb and stereo placement;
- drum-bus and beat-bus balance;
- deterministic variations, WAV printing and pack organization.

The system creates original procedural patterns and user-directed sample arrangements. It does not claim to reproduce a named producer's private chain, copy a released instrumental, or emulate a specific artist recording.

## 2. The ten studio rooms

| CRATE room | Hip-hop job | VentuSample baseline |
|---|---|---|
| **Crate Scout** | Inspect a user-selected sample crate, check source format and bounds, and keep source notes with the beat session. | Bounded PCM16 WAV import and pack manifests exist. Automatic BPM, key, transient and licence detection for arbitrary imports are not claimed. |
| **Chop Bench** | Split a source phrase into chops, pitch them and rebuild a loop. | Four-slice placement and semitone resampling are implemented. Automatic transient slicing is not claimed. |
| **Pocket Engine** | Set BPM, swing, snare drag/push and humanized timing. | BPM 60–190, swing 50–68%, snare offset −30…+30 ms, triplet rolls and micro-humanization are available. Cadence analysis reports scheduled generator events. |
| **Drum Room** | Shape kick, snare, hats and percussion into a coherent drum break. | Seeded drum generation, weighted drum voicing and restrained-hat Golden Era behaviour are implemented. |
| **Loop Lab** | Build sample-led loops, melody phrases and bass foundations. | Seeded Full Loop, Melody, Drums, Bass and One-Shot Kit targets are implemented across 14 documented styles. |
| **Arrangement Grid** | Work in bars, phrases, turns and alternate beat passes. | Bar-based rendering and deterministic variations are implemented. A full song timeline with verse, hook, bridge and automation lanes is not claimed. |
| **Texture Rack** | Add vinyl tone, grit, saturation, filtering, stereo character and other beat colour. | The offline DSP catalogue exposes 133 typed module identities over 21 reusable processing kernels. The catalogue does not represent 133 unrelated low-level algorithms. |
| **Beat Router** | Send the beat through controlled serial, parallel or hybrid effect paths. | The deterministic offline router uses eight bounded processors and seeded route selection. It does not hot-swap live effects. |
| **Mix Booth** | Balance low end, sample body, drum punch, stereo image and output peak. | Filters, saturation, linked stereo bus compression, peak/RMS/crest analysis and a sample-peak guard are implemented. LUFS mastering compliance and inter-sample true-peak certification are not claimed. |
| **Pressing Plant** | Print named WAV beats, variations and kits and keep their beat metadata together. | PCM16 WAV output and versioned pack manifests v1/v2 are implemented. A dedicated multi-stem export workflow is not claimed unless separately verified. |

## 3. The beatmaking route

A CRATE session follows the same path as a focused hip-hop production session:

1. **Pull the crate.** Select source WAVs and keep the original files untouched.
2. **Chop the phrase.** Place the supported four slices, choose pitch and set the source blend.
3. **Lock the pocket.** Choose BPM, swing, snare offset and humanization.
4. **Build the break.** Set drum density and let the selected style guide kick/snare/hat voicing.
5. **Lay the loop.** Generate or combine melody, bass and sample-led material.
6. **Turn the bars.** Render seeded variations and compare phrase changes.
7. **Colour the beat.** Add selected DSP modules or a bounded stochastic route.
8. **Balance the booth.** Check filters, compression, stereo, peak, RMS and crest.
9. **Print the record.** Export PCM16 WAV and the matching versioned pack manifest.
10. **Run the playback check.** Audition the prints, compare takes and keep the chosen version.

This is an offline production route. Rendering happens on a worker thread; the project does not claim live recording, live monitoring, device routing or real-time plug-in playback.

## 4. Beat Evolution: controlled variation, not self-modifying software

The former idea of autonomous adaptation becomes **Beat Evolution**.

Beat Evolution works with musical material and render settings only:

- inspect the current beat settings and available render metrics;
- identify a production target such as a tighter pocket, heavier kick/snare balance, denser hats or rougher sample colour;
- prepare alternative parameter combinations;
- render each pass from an explicit seed and variation index;
- compare cadence, peak, RMS, crest and offline spectral measurements where available;
- let the producer audition and keep the preferred take.

A variation never rewrites VentuSample source code, changes another project, installs a plug-in, scrapes sample libraries or uploads a beat. Determinism remains available: the same supported settings, seed and variation can be rendered again.

## 5. Crate, chop and module handling

Every source and processing module has a clear studio role.

- **Source crate:** user-selected audio files, bounded format metadata and user-maintained source/rights notes.
- **Chop map:** slice order, source pitch and blend values supported by the current four-slice workflow.
- **Pocket card:** BPM, swing, snare offset, triplet-roll and humanization settings.
- **Beat profile:** one of the documented hip-hop generation styles.
- **Texture chain:** ordered DSP module identities and their supported controls.
- **Beat print:** WAV output, render seed, variation and the existing pack metadata.

Only reviewed first-party handlers are executable. Plug-in manifest discovery remains metadata-only; a discovered DLL/SO or manifest entrypoint is not loaded automatically. Imported audio is untrusted data and must pass the existing size, frame, chunk and format checks.

## 6. The ten CRATE quality checks

A beat is ready for a clean print only after its production checks pass:

1. **Crate check** — source file is selected, readable, bounded and accompanied by useful provenance notes.
2. **Chop check** — slice placement, pitch and phrase order are intentional.
3. **Pocket check** — BPM, swing and snare placement serve the groove.
4. **Drum check** — kick, snare, hats and percussion leave each other room.
5. **Loop check** — bass, sample and melody sit in the same musical lane.
6. **Arrangement check** — bar count and variations are deliberate; no full-song structure is implied where none exists.
7. **Texture check** — saturation, filtering, delay, reverb and stereo width stay controlled.
8. **Mix check** — low-end balance, bus compression and output peak are reviewed.
9. **Print check** — WAV format and pack manifest match the selected render.
10. **Replay check** — the producer auditions the exported file and compares the chosen take against its alternatives.

A passing check must be backed by a reproducible render, a metric, a schema result or a listening decision. A feature name or a mock-up is not evidence that a beat tool is implemented.

## 7. The hip-hop profile crate

CRATE uses the styles already present in VentuSample rather than inventing a parallel genre system. The current preset contract includes:

- Boom Bap;
- East Coast;
- Harlem Dip-Trap;
- G-Unit;
- Modern Trap;
- Atlanta Trap;
- UK Drill;
- UK Grime;
- Memphis;
- Jazz Rap;
- G-Funk;
- Boom Trap;
- Cripple Trill;
- Billionaire Golden Era.

The Golden Era profile currently uses a fixed source-slice order of 0–2–1–3, kick/snare-weighted voicing and restrained hats. User BPM, swing and snare offset remain authoritative. These are original procedural profiles, not artist or recording emulations.

## 8. The beat pack and studio hand-offs

The existing file contracts remain the centre of every CRATE hand-off:

- **ventusample.preset.v1** stores validated beat settings.
- **ventusample.pack.v1** and **ventusample.pack.v2** carry generated WAV assets and their supported metadata.
- PCM16 stereo WAV remains the documented print format.
- Reaper pack exchange remains file-based and versioned. Reaper may inspect supported manifest metrics; it does not reinterpret or rewrite the audio payload.
- Pytyliki metrics remain an optional, separately verified hand-off; CRATE does not add a dependency solely to increase integration count.
- DAK or another companion surface may display beat-pack metadata in a future user-space workflow; this document does not assign audio-driver or device ownership.

No new preset or pack schema is implied by this specification. Any future contract change must be versioned, validated and accompanied by compatibility tests.

## 9. Studio safety and source respect

CRATE is local-first and offline by default.

- No background microphone capture or hidden recording.
- No hidden network transfer, sample scraping or cloud upload.
- No automatic overwrite of source WAVs or existing beat prints.
- No automatic loading of arbitrary DLL/SO files or plug-in entrypoints.
- No driver, firmware or system-audio installation.
- No automatic claim that a sample is royalty-free or cleared.
- No copying of third-party sample libraries without the user's rights and authorization.
- No specific-artist recording imitation presented as an original sample or cleared beat.

The producer chooses the source material and output location. Imported files remain untrusted until validated. Source provenance and licence notes are supplied by the user; VentuSample does not independently certify copyright ownership.

## 10. What is in the crate now, and what is still on the board

### Present in the inspected baseline

- C++20 offline beat and sample generation with 14 style profiles.
- Seeded render variations and bar-based phrase output.
- Four-slice source placement and semitone resampling.
- Swing, snare offset, triplet rolls and micro-humanization.
- Weighted drum voicing and the Golden Era source-slice pattern.
- Eight-effect deterministic offline routing with bounded wet, feedback, fades, RMS compensation and sample-peak protection.
- 133 typed DSP module identities implemented over 21 reusable kernels.
- Nineteen first-party plug-in registrations and a reviewed in-process handler API.
- Peak, RMS, crest and bounded offline spectrum measurements.
- PCM16 WAV output and versioned preset/pack contracts.

### Not claimed as shipped by CRATE

- Full multitrack song arrangement with verse/hook/bridge timeline.
- Automatic BPM, key or transient detection for arbitrary imported WAVs.
- Automatic sample-rights or licence verification.
- Live recording, real-time monitoring or hardware audio routing.
- VST/AU hosting or arbitrary third-party native plug-in loading.
- LUFS/EBU loudness certification or inter-sample true-peak certification.
- A full stem, acapella or multitrack export workflow.

These are separate beatmaking roadmap items. They must not be advertised as available until source code, contracts and repeatable tests prove them.

## 11. CRATE acceptance

A CRATE room is production-ready only when it has:

- a working audio path in the VentuSample codebase;
- a documented beatmaking control and parameter range;
- an explicit input/output contract;
- deterministic tests for repeatable results;
- integration tests for the connected beatmaking rooms;
- bounded source and render handling;
- a clear offline/live boundary;
- provenance and source-respect guidance;
- user-facing instructions and a working export route;
- evidence that the claimed beatmaking action actually works.

**CRATE's measure of success is a better beat: a stronger pocket, cleaner chops, clearer drums, musical loops, controlled texture and a repeatable print — with every studio move visible and every audio claim backed by the actual VentuSample build.**
