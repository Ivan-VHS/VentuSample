# Unified report envelope

All four Ivan-VHS tools can wrap a tool-specific JSON result in the same metadata contract, ivan-vhs.report.v1. The result payload remains owned by the originating tool; this envelope standardizes only provenance and limitations.

## Use

Run an existing JSON command and pipe its output into the wrapper:

    python tools/report_envelope.py --tool pytyliki --version 1.4.0 --commit "$GITHUB_SHA" < profile.json
    python tools/report_envelope.py --tool reaper --version 0.8.0 < audit.json
    python tools/report_envelope.py --tool ventusample --version 2.1.0 < render-manifest.json
    python tools/report_envelope.py --tool oplague --version 1.0.0 --input-file driver.zip < evidence.json

The wrapper reads one JSON value from standard input and writes one envelope to standard output. It does not alter the source result or the input file. The optional input-file argument records only its SHA-256 digest and the kind file; the local path is deliberately omitted. Hashing is streamed in 1 MiB chunks.

The commit is read from GITHUB_SHA or IVAN_VHS_COMMIT when --commit is not supplied. If no version is provided, the wrapper uses unknown rather than guessing. Each tool contributes default limitation statements; callers may append more with repeatable --limitation flags.

## Contract

The canonical JSON Schema is schemas/ivan-vhs.report.v1.schema.json. The result field is intentionally unconstrained by the shared schema and must be validated by the producing tool's own schema. Consumers should reject unknown envelope versions.

The envelope is a provenance aid, not a signature, security verdict, performance claim, chain of custody or proof of authorship. A SHA-256 digest proves byte identity only. The report records the platform of the wrapper process, which may differ from the machine that originally produced a previously saved result.
