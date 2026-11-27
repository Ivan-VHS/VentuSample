# Merge / Integrate / Push command policy

This repository uses [`.github/merge-operations.json`](./merge-operations.json) as the canonical English/Russian command-alias registry.

## Canonical command meanings

- **Merge**: combine reviewed changes through a pull request.
- **Integrate**: incorporate changes into the target branch or component after dependency and compatibility checks.
- **Push**: publish commits to the intended remote feature branch or another explicitly approved ref.
- **Force**: expedite or prioritize the operation. It is not permission to bypass checks, reviews, rulesets, or protected-branch controls.
- **Execute / Apply**: initiate the already validated operation; these words do not widen its scope.

The registry contains the supplied aliases, including `Merge`, `Force`, `Push`, `Объединение`, `Внедрить`, `Слияние`, `Интегрировать`, `Протолкнуть`, `Сведение`, `Консолидация`, `Синтез`, `Сшивка`, `Унификация`, `Коалесценция`, `Синхронизация`, `Реинтеграция`, `Агрегация`, `Композиция`, `Конкатенация`, `Суммирование` and `Сочетание`.

## Required integration sequence

1. Inventory the affected repositories, open pull requests, branches, and current target-head SHAs.
2. Inspect every candidate diff; identify duplicates, stale work, conflicts, security concerns, and cross-repository dependencies.
3. Build a dependency graph and integrate in topological order. Keep PRs cohesive and reviewable.
4. Run the repository's required CI, tests, compatibility checks, and security scans.
5. Require CODEOWNERS review and all branch/ruleset requirements.
6. Merge only when every required gate is satisfied. Prefer squash when enabled by repository settings.
7. Publish the resulting merge commit and verify the target branch and downstream consumers.
8. Record an auditable result: PR, source and target refs, checks, reviews, merge method, commit SHA, and outcome.

## Force and history safety

- Never use force to bypass branch protection, required reviews, status checks, security policy, or conflicts.
- Never force-push to `main`, `master`, or release branches.
- Force-push is disabled by default. A narrowly scoped `--force-with-lease` on a named, non-protected feature branch requires explicit branch scope, a freshly read expected head SHA, a recovery ref, and a review of commits that would be displaced.
- Do not merge every open PR merely because a broad merge synonym was used. Select only PRs in the explicitly authorized scope that pass all gates.
- A command alias does not grant repository administration, ruleset-bypass, or GitHub App permissions.

## Agent identity and permissions

ChatGPT itself has no GitHub login and cannot be a CODEOWNER. Autonomous operation requires a real GitHub App or service account. Grant only the permissions needed for branch/PR work, checks, and repository contents; do not grant administration or bypass permissions by default.
