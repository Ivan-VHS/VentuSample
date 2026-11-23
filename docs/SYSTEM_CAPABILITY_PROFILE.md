# System capability profile

This repository carries an evidence-based adaptation of the supplied 66-point Windows XP SP3 comparison. It is a **capability assessment contract**, not a request to reproduce Windows XP internals.

## Interpretation

The source list combines several kinds of statements: cumulative servicing, inherited Windows behavior, Windows-only networking and shell features, explicit absences, technical limits and repeated themes. Some claims may be version-dependent or historically imprecise. Therefore:

- the source is preserved as five numbered groups covering points 1–66;
- only portable engineering principles are considered for adoption;
- platform-specific capabilities are marked `not_applicable` when they fall outside this product's boundary;
- a missing capability is not described as implemented merely because it appears in a catalogue;
- no operating-system, kernel, driver, network, installer or hardware enforcement is inferred from a UI preference or design document.

## Machine-readable profile

The project assessment is stored in `.architecture/system-capabilities.v1.json`. Each control maps one source group to this repository's actual product scope and records:

- a status: `implemented`, `partial`, `planned`, `not_applicable` or `not_assessed`;
- a short assessment and explicit limitations;
- repository-relative evidence paths;
- test or verification paths where implementation is claimed.

The five source groups preserve the original numbering: security and policy (1–14), networking (15–28), installation and servicing (29–40), user interface and storage (41–54), and lifecycle and compatibility (55–66).

## Validation

Run from the repository root:

```sh
python tools/validate_system_capability_profile.py
python tools/validate_system_capability_profile.py --self-test
```

The validator checks schema identity, all 66 source-point references, contiguous group coverage, status semantics, evidence-path existence and negative test cases. It validates the **profile metadata and evidence links**; it does not prove that every documented behavior is correct. Product behavior remains subject to the repository's normal tests and review.

## Updating the profile

When a capability changes, update the profile in the same pull request as the implementation:

1. Inspect the implementation and relevant tests.
2. Choose the narrowest truthful status.
3. Add evidence paths that exist in this repository.
4. For `partial`, state what is still missing.
5. For `not_applicable`, explain the product boundary.
6. Add regression tests before claiming a new behavior is implemented.

Do not change `not_assessed` to `implemented` based only on roadmap text, a proposed API, a catalogue entry or a UI mock.
