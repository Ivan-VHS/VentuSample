# Engineering Evolution Framework

**Canonical shared framework:** [DuckOS — Engineering Evolution Framework](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/ENGINEERING_EVOLUTION_FRAMEWORK.md).

The 20-domain framework, shared practices and cross-project operating model are maintained centrally. This repository retains its own versioned adoption data:
- [Engineering evolution manifest](../engineering-evolution.v1.json)
- [JSON Schema](../schemas/engineering-evolution.v1.schema.json)

The capability-completeness section below remains local because this repository's validator checks its vocabulary and acceptance gates. It applies only to capabilities owned by VentuSample; local source, tests, CI and manifests determine actual maturity.

## Capability completeness standard

This section converts the additional 20 capability triples into one practical, evidence-led release standard. The terms are preserved verbatim as a vocabulary matrix; they are not twenty new product features and do not authorize scope expansion by themselves. Every project applies them to the capabilities it actually owns.

### Canonical vocabulary

| # | Principle | Completeness dimension | Functional outcome |
|---:|---|---|---|
| 1 | Расширяемость | Полноценность | Полнофункциональность |
| 2 | Масштабируемость | Завершённость | Всефункциональность |
| 3 | Модульность | Целостность | Функциональная полнота |
| 4 | Гибкость | Комплексность | Комплексная функциональность |
| 5 | Адаптивность | Всеохватность | Абсолютная функциональность |
| 6 | Универсальность | Исчерпываемость | Исчерпывающая функциональность |
| 7 | Модифицируемость | Совершенность | Всеобъемлющая функциональность |
| 8 | Настраиваемость | Самодостаточность | Целостная функциональность |
| 9 | Дополняемость | Функциональность | Системная функциональность |
| 10 | Развиваемость | Укомплектованность | Полная оснащённость |
| 11 | Совместимость | Оснащённость | Полная реализованность |
| 12 | Интегрируемость | Развитость | Всесторонняя реализация |
| 13 | Преобразуемость | Зрелость | Комплексная реализация |
| 14 | Вариативность | Полнота | Полная интеграция |
| 15 | Многофункциональность | Всесторонность | Всеохватная реализация |
| 16 | Перестраиваемость | Универсальность | Функциональная завершённость |
| 17 | Обновляемость | Состоятельность | Максимальная оснащённость |
| 18 | Реконфигурируемость | Законченность | Полная укомплектованность |
| 19 | Трансформируемость | Совершенство | Самодостаточность системы |
| 20 | Перспективность | Всеобъемлемость | Полная готовность системы |

### Capability lifecycle and maturity

Every user-visible, API, CLI, background, integration and native capability must have a stable identifier and an explicit scope. Track it as one of:

- **planned** — accepted scope, with no shipped implementation claim;
- **partial** — some behavior exists, but the declared acceptance criteria are not all met;
- **implemented** — all acceptance criteria for the stated scope are met and supported by code, tests and CI evidence;
- **not_applicable** — the capability is intentionally outside the product boundary, with a short rationale.

A catalogue entry, mock screen, placeholder, generated example, design document or green documentation check is not implementation evidence.
For repositories whose project-contract v1 schema accepts a narrower status set, keep planned and not_applicable items in the roadmap or implementation-status ledger. Do not force those states into the contract or widen an existing enum without an explicit schema migration.
 Never use “complete”, “absolute”, “all”, “maximum” or equivalent wording to imply capabilities beyond the explicitly declared scope. A product can be complete for its defined scope while clearly documenting future or excluded capabilities.

### Measurable acceptance gate

A capability may be promoted to **implemented** only when its record points to:

1. the owning source module or executable path;
2. automated tests for normal operation, invalid input, boundary conditions and relevant failure modes;
3. a CI job that builds or runs those tests on the supported platform matrix;
4. user or developer documentation covering invocation, configuration, outputs and limitations;
5. versioned schemas or interface contracts where data crosses a module or repository boundary.

The change must also demonstrate that it preserves existing behavior, has bounded resource use, reports failures deterministically, and does not silently broaden permissions or side effects. Performance claims require a reproducible baseline, workload, hardware, units and measurement method. Security-sensitive, destructive or privileged behavior requires explicit authorization, least privilege and a rollback or recovery path.

### Architecture, extension and integration rules

- Keep modules cohesive and independently testable. Public APIs, plug-in points, file formats and cross-project interfaces must be explicit, versioned and documented.
- Make extension possible through validated adapters, registries or configuration rather than hidden imports, hard-coded peer assumptions or unrestricted dynamic loading.
- Configuration needs a schema, defaults, bounds, migration behavior and compatibility rules. Invalid settings must fail clearly and safely.
- Scale only against measured resource budgets. Bound memory, CPU, file sizes, concurrency, retries, recursion, parsing and decompression where relevant.
- Treat imported files, packages, archives, media, serialized data and inspected binaries as untrusted. Inspection must not execute the inspected payload.
- Test compatibility in both directions where a contract is shared. Optional integrations remain opt-in and failures must not corrupt the standalone product.
- Prefer additive, reversible migrations. Remove or change a public behavior only with an explicit compatibility plan and regression tests.

### Project-specific application

The standard is universal; its implementation is deliberately product-specific:

| Project | Apply completeness to | Preserve this boundary |
|---|---|---|
| DuckOS | Mora shell, catalogues, native user-space tools and native-service contracts | Browser features are not proof of a bootable OS, kernel enforcement, compositor, driver stack or system service |
| Pytyliki | C11 processor discovery, bounded analysis, memory, metrics and native ABI | Hardware inspection remains read-only; capabilities absent from the source tree remain incomplete |
| Reaper | language frontend, reference runtime, IR/VM, tooling and integrations | Native compiler/self-hosting and hardware workbench stages remain partial until code and tests prove otherwise; native loading is explicit |
| OPlague | device/package inspection, compatibility analysis, reports and optional bridges | Inspection and dry-run planning do not install, enable, unload or modify drivers |
| VentuSample | deterministic offline generation, WAV import/export, DSP and pack exchange | Offline rendering is not real-time playback, DAW hosting or device control |

### Release gate

Before merging a feature, reviewers must be able to trace **requirement → design → source → tests → CI → documentation → versioned contract** without a missing link. The pull request must state what is implemented, what remains partial or planned, which compatibility guarantees apply, and how to reproduce verification. If evidence is missing, keep the status partial or planned and create a concrete follow-up; do not fill the gap with optimistic wording.

This gate supplements the existing 20-domain / 40-practice framework, project contracts, security policies and product-specific tests. It does not replace them, and it does not assert that every project must implement every technology.
