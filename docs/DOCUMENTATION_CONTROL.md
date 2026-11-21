# Documentation control

This file is the maintenance contract for the repository's human-readable and machine-readable documentation.

| # | Gate | Acceptance |
|---:|---|---|
| 1 | Evidence | Every implemented claim links to source, tests or a reproducible workflow. |
| 2 | Status | Separate implemented, partial, planned and unsupported behavior. |
| 3 | Privacy | Never publish secrets, private uploads, device identifiers, local paths or personal data. |
| 4 | Contract | State accepted formats, versions, limits, error behavior and exclusions. |
| 5 | Compatibility | Version breaking API, schema, manifest, CLI and runtime changes; include migration notes. |
| 6 | Visual evidence | Screenshots are genuine application captures, dated where useful, with alt text and no sensitive data. |
| 7 | Localization | Keep language variants aligned on capabilities, warnings, commands and versioned contracts. |
| 8 | Links | Verify relative links, anchors, commands and generated asset paths. |
| 9 | Security | Describe trust boundaries and safe failure; never infer enforcement from a catalog entry. |
| 10 | Release review | Re-run documented build/tests and review docs index plus root README before release. |

## Source-of-truth order

1. Executable source and automated tests define behavior.
2. Schemas and versioned machine-readable manifests define accepted data contracts.
3. CI workflows define repeatable validation commands.
4. Architecture and feature documents explain verified behavior and limitations.
5. README files summarize the project and link to detailed documentation.

If sources disagree, correct the documentation or code and tests together. Do not silently choose the more optimistic claim.

## Required status labels

- **Implemented:** code exists and focused tests exercise it.
- **Partial:** only the explicitly named subset exists.
- **Planned:** design or registry entry only.
- **Unsupported:** intentionally unavailable or outside the current product boundary.

A documentation page, screenshot, registry count or passing catalog validator is not implementation evidence by itself.

## Review checklist

- [ ] Claims match the current source branch.
- [ ] Numeric limits match code, schemas and tests.
- [ ] Build and test commands were executed or clearly labelled unverified.
- [ ] Security boundaries and failure modes are explicit.
- [ ] Links and image paths resolve.
- [ ] No private or sensitive data is included.
- [ ] AppGuard states are evidence-backed; no catalog-only control is marked active.
