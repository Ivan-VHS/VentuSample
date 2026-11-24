# Documentation control

**Scope:** VentuSample README family, audio/DSP documentation, manifests, schemas, screenshots and diagrams.

## Acceptance gates

| Gate | Requirement | Evidence |
|---|---|---|
| Evidence | Feature claims link to implementation, tests or schema | Source/test/schema paths |
| Maturity | Separate implemented, partial, planned and catalogued items | Feature matrix |
| Audio truth | Describe objective measurements separately from subjective sound | DSP contract and UI labels |
| Format bounds | Document PCM formats, rates, frames, sizes and numeric limits | Parser and schema |
| Determinism | State where seed/settings reproduce output and where variation is intentional | Generator tests |
| Plugin trust | Discovery metadata does not imply arbitrary code execution | Plugin manager source/tests |
| Rights | Do not claim ownership or licence clearance for imported/generated audio | Contribution and user guidance |
| Visual provenance | Real captures and authored diagrams are labelled separately | Asset review and alt text |
| Adaptive hero | Light/dark SVGs share dimensions and accessible fallback | SVG pair and picture block |
| Command accuracy | CMake, Qt and core-only commands match build options | CMakeLists and CI |
| Localization | All language variants preserve the same limits and commands | README comparison |
| Review hygiene | Links, Mermaid, images and git diff --check are checked | CI/local validation |

## Maturity vocabulary

- **Implemented:** behavior exists in current source.
- **Partial:** only a subset exists.
- **Specified / catalogued:** contract or registry entry exists; this does not prove runtime completion.
- **Planned:** future work without an available implementation.
- **Verified:** a named test/check passed on a stated revision and environment.

## Publication checklist

Update the README and the owning DSP/preset/plugin contract together when behavior changes. Keep the realtime/DAW distinction explicit. A module catalogue count is not a count of fully implemented processing kernels.
