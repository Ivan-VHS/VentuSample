# CareGuard v1 — repository support and workload safety

CareGuard adapts ideas from mental-wellbeing awareness into software-maintenance safeguards. It is an engineering-support layer, **not a mental-health product**.

## Shared runtime surface (all four repositories)

| API | Purpose | Safety boundary |
|---|---|---|
| `careguard_assess_project_load` | Score queue depth, failed checks, alert rate and worker-capacity pressure | Technical project metrics only |
| `careguard_review_language` | Flag stigmatizing wording and suggest neutral alternatives | Advisory; never silently rewrites text |
| `careguard_accept_help_request` | Accept a non-empty request for support | Help-seeking is valid input, not a weakness signal |
| `careguard_select_support_path` | Return a user-selected human-support route | Manual selection only; no contact or escalation is executed |
| `careguard_review_contextual_claim` | Require an evidence reference for evidence-backed claims | Missing evidence remains unverified |
| `careguard_check_clinical_boundary` | Block diagnosis and prescribing actions in this non-clinical application | No diagnosis, prescription, dosage or medication workflow |
| `careguard_allocate_resilience_buffer` | Reserve a bounded share of project capacity for recovery/maintenance | 0–50% reserve; deterministic calculation |

### Additional Python helpers (Reaper and OPlague only)

- `careguard_probe_sensitive_handler` smoke-tests a caller-supplied text handler against sensitive-topic probes and reports exceptions without retaining payloads.
- `careguard_record_context_event` records only an allowlisted engineering event, predefined role label and timestamp. Free-form personal identifiers are rejected.

## Renamed concept migration

| Original concept | CareGuard name / treatment |
|---|---|
| Depression-signal detection | `careguard_assess_project_load` — technical queue and CI pressure only; no depression detection |
| Stigma-tag removal | `careguard_review_language` — review suggestions, never destructive rewriting |
| Sensitive-keyword tremor check | `careguard_probe_sensitive_handler` |
| Dream/future-capacity tracking | Excluded from automatic measurement; personal optimism or recovery is never scored |
| Help request as weakness | `careguard_accept_help_request` |
| Family/social-bias claim review | `careguard_review_contextual_claim` — evidence and context are kept separate |
| Antidepressant decision log | Excluded; no medication or treatment history is stored |
| Grief/therapy history | Excluded; no personal event or treatment timeline is collected |
| Community discussion hook | Application-owned, opt-in feedback channel; not an automatic post or message |
| Provider/sponsor safeguards | Existing repository trust and supply-chain policy, plus evidence review |
| Onset questionnaire | Excluded by default; optional product-specific intake requires separate consent and data review |
| Motor lockdown / bed-rest protocol | `careguard_assess_project_load` — pause/degraded mode for software workloads only |
| Emergency contact / diagnosis | `careguard_select_support_path` and `careguard_check_clinical_boundary`; human choice, no automated contact or diagnosis |
| Episode-factor / somatic analysis | Technical incident and resource analysis only; no body or symptom metrics |
| Support pyramid | `careguard_select_support_path` exposes peer, trusted-person, professional and urgent-human choices without prescribing an order |
| Public cases / prevalence | Editorial awareness content only; statistics require independently verified sources |
| Resilience preallocation | `careguard_allocate_resilience_buffer` |
| Mood-based routing | Excluded; routes require an explicit user or operator choice |
| Prescription-only guard | `careguard_check_clinical_boundary` blocks clinical actions from this application |

## Privacy and safety invariants

- No inference or scoring of a person's mental state, symptoms, diagnosis, grief, therapy, medication or family circumstances.
- No background monitoring of individual contributors and no mood or health telemetry.
- No automatic message, emergency call, clinical referral or escalation.
- No clinical diagnosis, treatment recommendation, prescription or dosage handling.
- Sensitive language is reviewed with suggestions; the source string remains unchanged.
- Project workload assessments use only explicit technical counters and are advisory.
- Any user-facing support path is chosen by the person; it is not an automated triage result.

## Verification

The repository test suite covers empty and overloaded project snapshots, language review, help-request acceptance, manual routing, clinical-action blocking, evidence requirements and bounded capacity reserves. The C11 and C++20 variants implement the shared seven-function surface.
