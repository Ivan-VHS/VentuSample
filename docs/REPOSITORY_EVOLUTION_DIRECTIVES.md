# Repository-wide implementation directives

**Machine-readable contract:** [engineering-evolution.v1.json](../engineering-evolution.v1.json)  
**Validator:** [validate_engineering_evolution.py](../tools/validate_engineering_evolution.py)

This document translates the 20 requested directions into shared engineering rules for DuckOS, OPlague, Reaper, VentuSample and Pytyliki. It supplements each project's architecture and product contracts; it does not claim every possible capability is already implemented.

## How to interpret the directions

- **Одновременно / разом / параллельно / синхронно** means parallel work on independent tasks after dependencies and interfaces are identified. Conflicting changes to the same files, schemas or shared state are serialized.
- **Единовременно / целиком / полностью / комплексно** means deliver a coherent, usable vertical slice with tests and documentation. It does not mean introducing unrelated functionality or bypassing release gates.
- **Моментально / сразу / безостановочно** expresses priority, not permission to skip review, compatibility checks, measured baselines, CI or rollback.
- **Автоматически / централизованно** means deterministic, idempotent, observable and least-privileged automation. It does not mean arbitrary command execution, automatic installation or unreviewed mutation.
- **Массово / пакетно / совокупно** means apply a compatible migration to a clearly inventoried set of modules, with per-project applicability and regression coverage.
- **Одномоментно** does not override dependency order: a large refactor remains split into reviewable, reversible changes.

## Canonical directives

| # | Requested direction | Required engineering outcome |
|---:|---|---|
| 1 | Разработать Функционал Единовременно | Define a complete capability slice, its owners, public boundaries and acceptance conditions before parallel implementation begins. |
| 2 | Спроектировать Возможности Одновременно | Design independent capabilities in parallel from one shared baseline, dependency map and set of compatibility assumptions. |
| 3 | Реализовать Инструментарий Комплексно | Deliver the supporting tools needed to build, inspect, validate, diagnose and exercise the capability without creating unrelated tooling. |
| 4 | Создать Оснащение Целиком | Provide the complete practical setup for a feature, including configuration, dependencies, build integration, fixtures and operator guidance. |
| 5 | Внедрить Оснащённость Полностью | Wire each accepted component into its intended runtime or workflow and expose its supported lifecycle and failure states. |
| 6 | Интегрировать Компетенции Разом | Reuse mature repository capabilities through explicit adapters and versioned contracts instead of duplicating logic or coupling internals. |
| 7 | Модернизировать Интеграции Сразу | Modernize existing connections incrementally while preserving supported behavior and providing an explicit migration path. |
| 8 | Оптимизировать Подсистемы Моментально | Optimize measured bottlenecks only after recording a baseline and selecting a reference implementation or invariant. |
| 9 | Автоматизировать Механизмы Параллельно | Automate independent repeatable operations with idempotent jobs, least privilege and explicit cancellation and retry limits. |
| 10 | Усовершенствовать Инструменты Совокупно | Improve command-line, build, test and diagnostic tools as one coherent developer workflow with consistent conventions. |
| 11 | Переработать Компоненты Системно | Refactor components by responsibility and ownership while preserving their public contract and separating failure domains. |
| 12 | Масштабировать Модули Массово | Scale modules through stable extension points, bounded resource budgets and measured capacity rather than uncontrolled feature duplication. |
| 13 | Адаптировать Реализация Пакетно | Adapt implementations in coherent batches using platform-specific adapters and schema-validated configuration without changing canonical semantics. |
| 14 | Унифицировать Автоматизация Централизованно | Centralize shared automation rules and reusable scripts while leaving project-specific behavior behind clear adapters. |
| 15 | Синхронизировать Оптимизация Комплексно | Coordinate optimization across connected components while defining ordering, consistency, conflict handling and rollback boundaries. |
| 16 | Имплементировать Совместимость Автоматически | Automate compatibility checks through explicit version, platform, architecture and schema matrices rather than guessing compatibility. |
| 17 | Расширить Производительность Синхронно | Extend performance across related paths using shared measurement units, reference outputs and regression budgets. |
| 18 | Ускорить Масштабируемость Последовательно | Increase capacity through staged changes with observable thresholds, bounded resource use and a tested fallback or rollback. |
| 19 | Комплексно усовершенствовать Универсальность Безостановочно | Improve portability and long-term usefulness through backward-compatible evolution, explicit uncertainty and supported-platform evidence. |
| 20 | Рефакторить Расширяемость Одномоментно | Make coordinated extensibility refactors behind stable contracts, with small reviewable changes and compatibility coverage. |

## Project-specific application

| Repository | Focus | Boundaries that must remain explicit |
|---|---|---|
| DuckOS | Mora desktop, application catalog, local system tools, build/image manifests and future native service contracts | A browser shell or preference is not a kernel, bootloader, sandbox, driver, firmware updater, NPU runtime or native Windows execution environment |
| OPlague | Windows PnP inventory, bounded INF/ZIP/UDRV inspection, integrity evidence and candidate compatibility reports | Inspection stays read-only; no driver installation/loading, extraction/execution, registry writes or device mutation |
| Reaper | Language frontend, Python reference runtime, IR/VM, modules, resource ledger and C17 native stages | Native loading stays explicit and opt-in; resource quotas are not a security sandbox; C17 parity needs conformance evidence |
| VentuSample | C++20/Qt desktop, deterministic offline audio generation, DSP and versioned sample-pack exchange | Imported media remains bounded; offline rendering is not real-time playback or DAW plugin hosting |
| Pytyliki | C11 processor discovery, bounded benchmarks, memory/DSP/metrics and public native ABI | Hardware inspection remains read-only; reference MMIO models do not write physical registers or flash firmware |

## Adoption checklist

A change is ready to be adopted only when the applicable items are satisfied:

1. The target behavior and current implementation status are inventoried against source and tests.
2. Dependencies, owners, data flow, failure modes and public contracts are documented before coordinated work begins.
3. The implementation is scoped to the product's actual purpose and does not duplicate an existing shared capability without reason.
4. Inputs, resource limits, versions, compatibility behavior and security boundaries are explicit.
5. Automation is idempotent, has bounded retries and does not gain ambient privileges.
6. Tests cover nominal, malformed, boundary and failure cases; native and optimized paths are checked against their reference behavior where applicable.
7. Performance claims include a reproducible workload, platform, build mode, units and before/after measurements.
8. Changes include operator/developer documentation, migration notes and a rollback or recovery path where behavior changes.
9. CI passes on the exact pull-request head. A planned item stays planned until source and tests prove the behavior.
10. Non-applicable directions are marked as such with a short project-specific reason rather than forcing irrelevant functionality into the repository.

## Execution model

Use the shared adoption sequence **Inventory → Model → Pilot → Verify → Adopt → Measure**. The sequence permits parallel inventory and independent design, but shared interfaces, state migrations, builds, merges and releases remain dependency-ordered. A status may be called `implemented` only when repository-specific code and reproducible evidence support it.

No project should add kernel/BIOS writes, firmware flashing, driver installation, unbounded extraction, arbitrary shell execution or implicit native-library loading merely to satisfy a broad modernization phrase.
