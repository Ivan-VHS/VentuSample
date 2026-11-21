# Project stewardship and anonymous operating principles

**Status:** normative project-governance guidance. This document defines operational and communication expectations; it does not claim new audio-engine behavior or commercial validation.

## Purpose

Deliver expressive, dependable sample-generation tools through disciplined craft, reproducible output, respectful collaboration and truthful product communication. Musical usefulness and technical integrity matter more than attention, inflated genre claims or promotional volume.

## Operating principles

1. **Creative intent with measurable delivery.** Every generation control should have a clear musical or technical purpose. Quality metrics inform review; they do not replace listening or user judgment.
2. **Discipline and repeatability.** Keep parameters, seed, variation, output format and known limitations explicit. A result should be reproducible when the declared inputs are unchanged.
3. **Clear decision ownership.** Separate generator, DSP, desktop UI, schema and release responsibilities. A personal relationship or informal status must not silently change the technical contract.
4. **Evidence over image.** Describe what the renderer actually produces, which formats it accepts and which processing stages are present. Do not imply access to proprietary recordings or endorsement by a performer.
5. **Respectful creative feedback.** Critique the result and its behavior, not the identity, age, gender, background or personal life of its creator.
6. **Sustainable production.** Keep batches bounded, cancellation and partial failure visible, and the work documented so maintenance does not depend on continuous availability of one contributor.

## Decision record

For changes to generation algorithms, audio processing, musical controls, metadata, import/export or licensing assumptions, record:

- **Creative or user problem and expected outcome.**
- **Decision owner and affected audio/data boundary.**
- **Parameters, seed, sample rate, channel layout and output format.**
- **Evidence:** regression fixtures, measured peaks/RMS/crest, listening notes and known limitations.
- **Alternatives and trade-offs.**
- **Source-rights review, failure handling and rollback.**

A subjective sound-design choice should be labelled as a design decision, not disguised as an objective benchmark. A change to an exported manifest or pack contract requires explicit versioning and compatibility notes.

## Execution rhythm

1. **Intake:** classify requests as sound design, musical structure, DSP, import/export, desktop UX, interoperability or release work.
2. **Specify:** define parameter ranges, deterministic behavior, output invariants and expected audible differences.
3. **Render:** use bounded input and output sizes; keep the desktop responsive and report failures per item.
4. **Evaluate:** run deterministic tests and objective measurements, then perform listening checks for musical intent.
5. **Package:** preserve parameter snapshots, file metadata, schema version and provenance needed for reproduction.
6. **Review:** update guides, examples and release notes; capture feedback and known gaps as owned follow-up.

Keep work-in-progress and generation batches bounded. A failed export must not silently overwrite unrelated samples or leave the user believing that a complete pack was created.

## Communication, feedback and public credibility

- Separate objective properties (format, duration, level, sample rate, seed and metadata) from subjective judgments (groove, character, musicality and preference).
- Do not claim that peak/RMS/crest values establish artistic quality, loudness compliance or compatibility with every playback chain.
- Do not claim imported audio has a detected key unless a documented detector actually performed and validated that analysis.
- Treat sample rights and licensing as explicit release requirements. A cryptographic digest identifies bytes; it does not establish ownership or permission to redistribute.
- Prioritize feedback by audible impact, reproducibility, user reach, confidence and implementation cost—not by views or online controversy.
- Explain errors in practical terms, including unsupported formats, size limits and recovery steps.
- Avoid manufactured hype, fake scarcity, misleading artist associations and promises of demand, sales or profitability without evidence.

## Incident and conflict protocol

Use **stabilize → assess → contain → communicate → recover → review**. Stop a faulty batch, preserve the seed and parameter snapshot, identify affected files, prevent accidental overwrite, explain the confirmed impact and provide a clean regeneration path. Verify the fix with deterministic fixtures and listening checks, then record a blameless retrospective.

## Sustainable stewardship

Maintain a risk register for imported media, licensing, parameter drift, schema compatibility, clipping, incomplete exports and desktop responsiveness. Assign an owner, mitigation and review date. Keep private family details and personal stories out of sample metadata, filenames, demo packs and screenshots unless disclosure is deliberate, necessary and authorized.

Respect creative differences while keeping professional boundaries clear. Review output against agreed criteria; do not turn artistic disagreement into a personal dispute or use status as a substitute for evidence.

## VentuSample-specific acceptance gates

- Fixed seed plus the same validated parameters must produce repeatable output within the documented deterministic contract.
- Every export preserves the versioned pack schema and the parameters needed to reproduce it.
- WAV imports remain bounded by documented format, size, chunk and frame limits; unsupported formats are rejected clearly.
- Tonal labels describe the generator's selected key/mode metadata. Do not imply automatic key detection for imported source material unless that capability is actually implemented and tested.
- Mastering and level measurements remain explicit. A sample-peak ceiling does not imply true-peak control, platform loudness compliance or guaranteed playback compatibility.
- Offline operation and explicit native-library boundaries remain intact.
- Source-sample rights, distribution permission and attribution are reviewed separately from technical file integrity.

## Release review checklist

- [ ] The change has a clear musical or workflow purpose.
- [ ] Parameter bounds, deterministic behavior and failure handling are tested.
- [ ] Audio is evaluated with both measurements and listening where appropriate.
- [ ] Export names, manifest fields and schema versions are consistent.
- [ ] Rights, provenance and privacy concerns are addressed.
- [ ] User-facing claims distinguish fact from subjective assessment.
- [ ] Known limitations and regeneration/recovery steps are documented.

**Interpretation rule:** this standard does not change generator semantics, confer sample rights, establish market demand or convert objective audio measurements into a universal artistic score.
