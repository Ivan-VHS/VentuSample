# Source audio integration and rights boundary

## Supported input

VentuSample accepts local PCM16 WAV source audio in mono or stereo. The import path is bounded by the documented file-size, frame, channel and format checks in the generator. Unsupported codecs and sample encodings must be rejected rather than silently decoded or relabelled.

The supported workflow is:

1. Select a local WAV file through the application.
2. Validate its RIFF/WAVE structure, PCM format, channel layout and resource bounds.
3. Choose a source region and use the supported four-slice placement and semitone-resampling workflow.
4. Render the new material through the offline generator.
5. Export generated PCM16 WAV assets with the versioned pack manifest.

The importer is not a general-purpose audio decoder. It does not imply support for MP3, FLAC, 24-bit PCM, floating-point WAV or arbitrary compressed audio.

## Provenance and reproducibility

Keep a record of the source file, source owner or license, the operator's permission basis, the imported regions and the generation parameters used for the output. A fixed generator seed and parameter set support repeatable procedural output; they do not prove ownership of the source recording.

Where the export workflow does not preserve a particular provenance field, record it separately with the pack's project notes. Never state that a generated pack is commercially cleared solely because it was rendered locally or passed the WAV/manifest validator.

## Rights and commercial distribution

The operator is responsible for ensuring they have the rights and permissions needed to import, transform, distribute and commercially use every source recording and third-party asset. A local file selection is not a rights grant. VentuSample does not identify copyright owners, authenticate source licenses, clear samples, or grant rights to source material.

Before distributing a pack commercially:

- verify the source recording and composition rights applicable to the intended territory and use;
- review any sample-library, loop-pack or third-party content license;
- preserve required attribution and license notices;
- confirm that the generated output and included metadata comply with those terms;
- keep a review record for any external distribution or sale.

SHA-256 can identify file bytes and help compare artifacts. It does not establish the identity of a publisher, ownership of a recording, license validity or legal clearance.

## Product boundary

VentuSample is an offline procedural generator and sample-pack exporter. This document does not describe a DAW, real-time playback engine, plugin host, source-audio marketplace or automated legal-clearance service. The sample-pack format remains governed by `schemas/ventusample.pack.v1.schema.json`.
