# Human Pull Request and Branch Style

This guide sets one clear writing and naming style for work in this repository and across the Ivan-VHS project family.

## Write like an engineer

Write for another developer who needs to understand the change and review it. Use ordinary, direct language. Say what changed, why it matters, and what was actually checked.

- Prefer concrete verbs and the name of the affected subsystem.
- Keep one pull request focused on one coherent result.
- Explain dependencies and merge order when work is stacked across branches or repositories.
- State what is not implemented when that boundary could be misunderstood.
- Report test and CI results accurately. Use `not run`, `pending`, or `failed` where appropriate.
- Do not claim completion, compatibility, security, performance or production readiness without evidence.
- Avoid promotional language, inflated adjectives, repeated boilerplate and generic claims such as “various improvements”.
- Keep technical detail when it helps review. Human wording must not weaken precision.

## Pull request titles

Use a concise Conventional Commit title:

`<type>(<scope>): <clear imperative result>`

Examples:

- `feat(parser): preserve source spans through typed lowering`
- `fix(audio): reject malformed WAV chunk lengths`
- `docs(boot): explain the current boot boundary`
- `security(devices): keep device inspection read-only`
- `style(mora): align focus and motion tokens`

Choose the type that describes the change:

- `feat` — adds user-visible or developer-facing capability.
- `fix` — corrects incorrect behavior.
- `security` — adds or changes a security control.
- `docs` — documentation or specification only.
- `refactor` — changes internal structure without changing intended behavior.
- `perf` — measurable performance work.
- `test` — test-only work.
- `ci` — build, workflow or automation changes.
- `style` — visual or interaction design changes.
- `chore` — maintenance that does not fit the types above.

Use a scope that names the real subsystem, not a generic word such as `misc` or `update`. Keep the title specific and readable; do not force extra words into it.

## Pull request description

Start with one or two sentences in plain language: what the change delivers and why it belongs here. Use only the sections that help explain the work:

### What changed
Describe the behavior, interfaces, files or user workflow that changed. Prefer a short list for several distinct items.

### Why
Name the problem or limitation this work addresses.

### Verification
List the exact commands, CI runs and relevant results. Separate checks that passed from checks that are pending or were not run.

### Compatibility and safety
Explain affected public contracts, formats, APIs, data, permissions, resource limits and platform boundaries. State what remains unsupported.

### Dependencies
Link related pull requests and branches. For stacked work, state the required integration order and the intended base branch.

Do not keep an empty section just to satisfy a template. A short, accurate description is better than a long, repetitive one.

## Branch names

Use lower-case English kebab-case so branch names work consistently in Git, scripts and CI:

`<kind>/<specific-purpose>[-YYYY-MM-DD]`

Use one primary prefix for each branch:

| Prefix | Use |
| --- | --- |
| `feat/` | New functionality |
| `fix/` | A normal bug fix |
| `hotfix/` | An urgent production correction |
| `work/` | A coherent workstream or topic |
| `experiment/` | An experiment or prototype |
| `integration/` | Cross-module or cross-repository integration |
| `release/` | Release preparation |
| `security/` | Security and defensive controls |
| `chore/` | Repository maintenance |
| `docs/` | Documentation |
| `refactor/` | Internal restructuring |
| `perf/` | Performance work |
| `test/` | Test-only work |
| `ci/` | CI and build automation |
| `spike/` | A time-boxed technical investigation |
| `variant/` | An alternative implementation |

Examples:

- `feat/typed-source-spans`
- `fix/bounded-pool-metadata`
- `integration/reaper-device-evidence`
- `security/readonly-device-inspection`
- `docs/localized-readme-refresh`

Name the outcome, component or technical subject. Avoid vague names such as `new-feature`, `misc-update`, `final-version` or `all-fixes`. Add a date only when it distinguishes a recurring workstream or snapshot.

Keep `backup/` and `archive/` branches clearly identifiable and do not treat them as ordinary work branches. Do not delete them automatically. Do not rename an active branch until its open PRs, dependent branches, automation and external references have been checked.

## Integration discipline

A branch is a line of work, not an instruction to overwrite another line. Review the diff and dependency graph, preserve the intended base, run the required checks and merge compatible work in dependency order. Keep history and recovery points available until the integration is verified.
