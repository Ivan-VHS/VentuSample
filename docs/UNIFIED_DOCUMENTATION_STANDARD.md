# Ivan-VHS Unified Documentation and Semantics Standard

**Document ID:** `IVAN-VHS-DOCS-STD-001`  
**Version:** 1.0  
**Scope:** all Ivan-VHS repositories, their documentation, machine-readable project contracts and cross-project integrations  
**Normative language:** MUST, SHOULD and MAY are used in their ordinary standards-writing sense.

This standard establishes one shared vocabulary and one consistent way to describe implementation maturity, runtime state, verification results, evidence and project boundaries. It does not make the repositories identical products: each project keeps its own architecture, domain terminology, interfaces and technical constraints.

## 1. Authority and source of truth

Use the following authority for the type of fact being described:

1. **Source code** defines the behavior currently implemented.
2. **Tests and CI results** show which behavior was exercised, with the limits of the tested platform, configuration and revision.
3. **Versioned schemas, protocol definitions and ABI declarations** define machine-readable and externally visible contracts.
4. **Build files and release workflows** define supported build and packaging procedures.
5. **Documentation** explains the behavior, constraints and supported workflows; it MUST agree with the sources above.
6. **Diagrams, screenshots and catalogues** are explanatory evidence or inventories. They are not implementation proof by themselves.

When these sources disagree, do not silently choose the most optimistic description. Record the discrepancy, check the current source and tests, then correct the lower-authority description. A roadmap, registry entry, mock-up, reserved identifier or design proposal MUST NOT be described as a shipped capability.

A document may be authoritative for a design decision or a normative contract. That does not make it proof that the contract is implemented.

## 2. Canonical terminology

Use these terms consistently:

| Term | Meaning |
|---|---|
| **Repository** | The version-controlled code and assets in one Git repository. |
| **Project** | The product and engineering work represented by a repository. |
| **Product** | The user-facing application, tool or library delivered by a project. |
| **Component** | A bounded implementation unit, such as a service, module, process, library or UI subsystem. |
| **Capability** | A user-visible or machine-observable behavior that can be stated and verified independently. |
| **Feature** | A user-oriented grouping of one or more capabilities. A feature is not automatically fully implemented. |
| **Contract** | A normative statement of inputs, outputs, invariants, limits, errors and compatibility expectations. |
| **Interface** | A boundary through which components or projects exchange calls or data. |
| **Schema** | A machine-readable description of a structured data format and its validation constraints. |
| **API** | A source-level or message-level callable interface. |
| **ABI** | A binary-level calling, layout, symbol and compatibility contract. |
| **Runtime state** | The current operational condition of a component or process. |
| **Implementation maturity** | How far a capability has progressed from specification to verified behavior. |
| **Verification evidence** | A reproducible test, CI result, capture, benchmark or review supporting a specific claim. |
| **Integration policy** | Whether a connection to another component or project is required, optional or explicitly enabled. |

Use **capability** for atomic status claims. Avoid assigning one maturity value to a broad product area when its constituent capabilities have different maturity.

## 3. Keep four status dimensions separate

A bare word such as “status” is ambiguous. New documentation SHOULD name the dimension explicitly: `maturity_status`, `runtime_state`, `verification_status` or `integration_policy`.

### 3.1 Implementation maturity

Use only these canonical identifiers for capability maturity:

| Identifier | Meaning | Minimum evidence |
|---|---|---|
| `not_assessed` | The capability has not been audited against current source and tests. | No implementation claim may be made. |
| `specified` | A written contract, design or schema exists; executable behavior is not claimed. | Link to the specification or schema. |
| `planned` | Work is accepted as a future target but is not implemented. | State the intended scope and acceptance conditions. |
| `partial` | A working subset exists; missing behavior and boundaries are explicit. | Source paths and tests for the implemented subset. |
| `implemented` | The described behavior exists in the product and is connected to a real workflow. | Source paths and a reproducible way to exercise it. |
| `verified` | The implemented capability meets its stated acceptance criteria in the named environment and revision. | Passing relevant tests or reproducible checks, with environment and revision recorded. |
| `deprecated` | The capability remains for compatibility but SHOULD NOT be used for new work. | Reason, replacement and migration/removal policy. |
| `unsupported` | The project explicitly does not provide or support the capability. | A clear scope or architecture boundary. |
| `not_applicable` | The capability does not apply to this project or component by design. | A brief reason tied to project scope. |

Rules:

- `verified` implies `implemented`; it is not a synonym for “documented”.
- `specified` and `planned` are not interchangeable. A specification describes a contract; a plan records accepted future work.
- `partial` MUST say what works and what does not.
- `implemented` MUST NOT imply complete parity, production readiness, certification or universal platform support.
- `deprecated` is not the same as removed. Use `unsupported` when the capability is not provided.
- `not_assessed` and `unknown` MUST NOT be presented as negative test results.
- A catalogue count, feature ID or registry entry MUST NOT raise a capability's maturity.

### 3.2 Runtime state

Runtime state describes what a component is doing now, not how complete its implementation is. Where a shared lifecycle is suitable, use these identifiers:

| Identifier | Meaning |
|---|---|
| `uninitialized` | The component has not started initialization. |
| `initializing` | Initialization is in progress. |
| `ready` | Initialization succeeded and the component can accept work. |
| `running` | The component is processing an operation or workload. |
| `degraded` | The component remains usable, but a documented capability or dependency is unavailable. |
| `stopping` | Shutdown is in progress. |
| `stopped` | The component is not running and has shut down normally. |
| `failed` | The component cannot continue the current lifecycle or operation. |
| `unknown` | The state was not observed, is stale or cannot be determined. |

A component MAY define domain-specific substates, such as audio-render, package-inspection or device-discovery states. It MUST document their meaning and map them to the shared lifecycle where that mapping is meaningful. Do not force irrelevant states onto a component.

For every non-trivial state machine, document the state owner, entry conditions, exit conditions, valid transitions, timeout behavior and failure recovery. Do not use `implemented`, `planned` or `verified` as runtime states.

### 3.3 Verification result

Use these identifiers for a test, build, check or review execution:

| Identifier | Meaning |
|---|---|
| `not_run` | The check has not been executed. |
| `passed` | The check completed and met its stated criteria. |
| `failed` | The check executed and did not meet its criteria. |
| `blocked` | The check could not be completed because a prerequisite or environment was unavailable. |
| `skipped` | The check was intentionally not run under the documented conditions. |
| `not_applicable` | The check does not apply to this target. |

A missing dependency is `blocked`, not `passed`. A skipped test is not evidence of success. A passing test applies only to the behavior, revision, platform and configuration it actually exercised.

### 3.4 Integration policy and availability

Use integration policy separately from maturity:

- `required` — the documented workflow requires the integration.
- `optional` — the integration is supported but not necessary for the core product.
- `opt_in` — the operator or caller must explicitly enable or select it.
- `disabled` — the integration is deliberately disabled in the described configuration.
- `not_applicable` — no such integration exists in the project scope.

Describe current availability separately: available, unavailable in this environment, disabled by configuration, failed, or unknown. A capability may be implemented while an optional integration is disabled. An integration may be planned without an implementation. The words `optional` and `opt_in` MUST NOT be used as implementation-maturity values.

### 3.5 Legacy manifest fields

Some existing project-contract v1 files use a generic `status` property in more than one object type. Interpret it according to that object's schema and context; do not infer that every `status` field means capability maturity.

This standard does not silently change existing manifest schemas or field meanings. Any schema migration MUST be versioned, validated by the repository's contract checks and documented with a compatibility path. New schemas SHOULD use explicit property names for each status dimension.

## 4. Evidence and claims

Every material implementation claim SHOULD point to the smallest useful evidence set:

- source files or public declarations;
- focused unit or integration tests;
- CI workflow and the tested platform/configuration;
- schema validation or protocol conformance checks;
- a real application capture for a visual/UI claim;
- a benchmark with hardware, OS, build mode, workload and measurement method;
- a manual review record when automation cannot establish the claim.

Use precise wording:

- **“Implemented”** means the behavior exists and is wired into a real path.
- **“Verified”** means the stated acceptance check passed; name the check and environment.
- **“Measured”** means an actual measurement was performed; report conditions and method.
- **“Simulated”** means the result comes from a model or simulation, not physical hardware or a live system.
- **“Example” / “demo”** means illustrative input or output, not live production data.
- **“Read-only”** applies only to the specific operations shown to avoid writes.
- **“Safe”, “secure”, “compatible”, “production-ready” and “certified”** require a defined threat, compatibility or operational scope and evidence. Do not use them as decorative adjectives.

Performance claims MUST NOT be based only on a proposed algorithm, a synthetic example or a single unqualified timing. Hardware and operating-system observations MUST state platform restrictions. A screenshot proves the visible state of that capture, not hidden runtime behavior.

## 5. Documentation structure

Use a predictable documentation hierarchy:

| Location | Responsibility |
|---|---|
| Root `README.md` | Product identity, verified scope, current capabilities, boundaries, quick start and links to authoritative docs. |
| Localized root READMEs | Equivalent scope, maturity, warnings, commands and links in the target language. |
| `docs/README.md` | Complete, maintained navigation index for technical documentation. |
| `docs/ARCHITECTURE.md` | Actual components, data/control flow, dependencies, trust boundaries and failure paths. |
| `docs/FEATURES.md` | Capability-level maturity matrix, constraints and evidence. |
| `docs/ENGINEERING.md` | Build, test, review, security and release gates. |
| Contract/API/ABI/schema documents | Exact versioned external behavior and compatibility rules. |
| User/operator guides | Reproducible tasks, prerequisites, expected outcomes and recovery steps. |
| Catalogues and roadmaps | Clearly labelled inventory or future work; never a substitute for feature documentation. |

Normative or frequently changing technical documents SHOULD include: purpose, scope, maturity where relevant, current behavior, explicit non-goals/boundaries, evidence, verification instructions and related links. Do not add empty headings merely to satisfy a template.

The documentation index MUST link the current canonical standard and all maintained normative documents. Remove or repair stale links when files are renamed or deleted. Avoid duplicate competing descriptions of the same contract: choose one canonical document and link to it.

## 6. Language and localization

English identifiers in code, schemas and machine-readable status values remain canonical and stable. Translations MAY localize prose, headings and examples, but MUST preserve:

- the same capability scope and maturity;
- the same runtime-state and verification meanings;
- warnings, limitations, safety boundaries and non-goals;
- command names, file paths, schema keys, enum values and version identifiers;
- the same evidence strength.

A localized README MUST NOT claim more than the canonical README. Keep technical terms consistent across English, Ukrainian, Russian and Simplified Chinese documentation. When a term has no safe direct translation, retain the canonical identifier and explain it once.

## 7. Cross-project contracts

Projects remain independently buildable and retain ownership of their own implementation. A federation or peer integration MUST state:

- provider and consumer;
- interface name and explicit version;
- transport and data direction;
- opt-in/required policy;
- schema/ABI validation and evidence;
- compatibility and failure behavior;
- what data is exchanged and what the receiving project does not do.

Do not imply that one repository's capability is available in another simply because a schema, adapter stub or catalogue entry exists. Document the real call path and its trust boundary. Keep optional native loading explicit and identify the trusted library or executable selected by the operator.

## 8. Change acceptance checklist

Before merging a documentation change, check:

1. Does the text describe behavior present in the current source, or clearly label it as specified/planned?
2. Is each maturity claim attached to an atomic capability and supported by evidence?
3. Are implementation maturity, runtime state, verification result and integration policy kept separate?
4. Are unknown, unavailable, disabled, unsupported and not-applicable cases distinguished?
5. Do limits, platform requirements, failure modes and non-goals remain visible?
6. Do README, docs index, localized versions, schemas and examples agree?
7. Are commands and paths real and reproducible on the stated target?
8. Are screenshots, diagrams, demo data and benchmarks labelled accurately?
9. Are cross-project interfaces versioned and opt-in behavior explicit?
10. Were stale links and contradictory duplicate claims removed?

A documentation-only change does not change implementation maturity. A source change that changes behavior MUST trigger a review of the affected README, feature matrix, architecture page, user guide, schema and localized documentation.

---

**Attribution:** Preserve the authorship and copyright notices already present in each repository. Attribute Ivan-VHS as project author/developer where appropriate; do not replace the authorship of other contributors or third-party works.
