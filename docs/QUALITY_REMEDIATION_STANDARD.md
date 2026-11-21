# Comprehensive correction, coverage and defect standard

## Purpose and operating boundary

This standard applies the supplied 20 triplets as a practical remediation system: a correction method, a coverage principle and a defect class. The terms are preserved verbatim, including the repeated **«Исчерпывающесть»** in rows 5 and 18.

The matrix is a review and verification contract, not a claim that every defect has already been eliminated. «Безошибочность» is an aspirational release objective, never a guarantee of zero defects. «Безграничность» means broad coverage, not unbounded memory, CPU, retries or execution time. Resource and safety limits remain explicit.

## 20 × 3 remediation matrix

| # | Метод исправления | Принцип охвата | Класс дефекта | Acceptance criterion |
|---:|---|---|---|---|
| 1 | Исправление | Всеобъемлемость | Ошибки | Reproduce the defect, isolate its root cause, add a regression check, and verify the corrected behavior at the affected entry point. |
| 2 | Корректировка | Всесторонность | Погрешности | Check boundaries, units, tolerances, rounding and measurement assumptions from more than one independent path. |
| 3 | Устранение | Комплексность | Неточности | Remove the underlying cause across dependent components; do not hide symptoms with local workarounds. |
| 4 | Поправка | Полнота | Недочёты | Close missing acceptance conditions across implementation, tests, documentation, configuration and distribution artifacts. |
| 5 | Налаживание | Исчерпывающесть | Просчёты | Exercise the complete user workflow, including invalid input, cancellation, failure and recovery branches. |
| 6 | Отладка | Универсальность | Отклонения | Run deterministic diagnostics for supported platforms and configurations; report unsupported combinations explicitly. |
| 7 | Доработка | Многогранность | Дефекты | Review functional behavior, security, resource use, accessibility, compatibility and documentation as one change set. |
| 8 | Оптимизация | Масштабность | Изъяны | Measure a reproducible baseline before optimizing and prove that correctness, safety and output quality do not regress. |
| 9 | Реконструкция | Охватность | Недостатки | Restructure components only with explicit interface contracts, migration notes and compatibility regression checks. |
| 10 | Модернизация | Многоаспектность | Оплошности | Update toolchains, platforms and dependencies deliberately, then run the affected build and regression matrix. |
| 11 | Обновление | Многопрофильность | Промахи | Synchronize supported profiles, operating modes, locales and configurations so they do not drift independently. |
| 12 | Ревизия | Разносторонность | Сбои | Audit implementation, tests, diagnostics, release workflows and evidence links; record findings with reproducible references. |
| 13 | Восстановление | Всеохватность | Нарушения | Recover from partial failures without corrupting existing data; verify idempotency and atomicity where relevant. |
| 14 | Регулировка | Целостность | Упущения | Keep defaults, thresholds, quotas, timeouts and policy boundaries consistent with the documented operating envelope. |
| 15 | Совершенствование | Совокупность | Несоответствия | Verify end-to-end conformance from the public interface through implementation and persisted/exported results. |
| 16 | Рефакторинг | Глобальность | Дисфункции | Remove harmful duplication and dependency cycles while preserving public contracts and observable behavior. |
| 17 | Стабилизация | Безграничность | Неисправности | Contain failures with explicit upper bounds for memory, CPU, retries, input sizes and execution time. |
| 18 | Согласование | Исчерпывающесть | Оговорки | Resolve mismatches between source code, schemas, APIs, specifications, tests and user-facing documentation. |
| 19 | Унификация | Фундаментальность | Ляпы | Use canonical data formats and versioned interfaces for shared capabilities without erasing project-specific boundaries. |
| 20 | Безошибочность | Энциклопедичность | Деформации | Gate releases on relevant regression checks, clean build artifacts, synchronized documentation and reviewable evidence. |

## Required correction lifecycle

1. **Reproduce:** capture the input, environment, expected result and actual result. A defect without a reproducible case remains an open investigation.
2. **Classify:** record affected component, severity, user impact, data/safety impact and the relevant matrix row.
3. **Isolate:** identify the earliest incorrect state or violated contract; distinguish root cause from downstream symptoms.
4. **Correct:** make the smallest coherent change that fixes the cause without weakening safety or compatibility boundaries.
5. **Regress:** add a focused test, then run the affected component suite and repository CI.
6. **Reconcile:** update schemas, APIs, examples, release notes and supported-platform statements affected by the change.
7. **Verify:** review the diff, test evidence and remaining limitations before closing the defect.

## Severity and release gates

| Severity | Examples | Release handling |
|---|---|---|
| **P0 — Critical** | Unsafe side effects, security boundary bypass, data loss or corruption | Block release; contain and resolve before merge |
| **P1 — High** | Crashes, incorrect core output, broken persistence, nondeterministic results or major compatibility regression | Block affected release path until fixed and covered by regression tests |
| **P2 — Moderate** | Incorrect edge-case behavior, incomplete workflow, diagnostic mismatch or supported-platform regression | Fix before the affected capability is declared complete; document any accepted exception |
| **P3 — Low** | Non-critical usability, wording, presentation or maintenance issue | Track with an owner and acceptance criterion; do not misreport as resolved |

## Definition of done

A defect is resolved only when its reproduction is recorded, the cause is addressed, a regression test covers the failure, relevant checks pass, and affected documentation/contracts are synchronized. A passing evidence-path check alone does not prove the referenced behavior is correct.

The repository profile `quality-remediation.v1.json` records the 20 terms, project-specific targets, acceptance criteria and repository-local evidence paths. Run:

```bash
python -m unittest discover -s tests -p test_quality_remediation.py -v
python tools/quality_remediation.py
python tools/quality_remediation.py --format json
```

The standard-library evaluator validates the profile, exact term order, safe repository-relative paths and evidence-file presence. It does not execute product code, assess the semantic quality of a source file, perform a full static analysis or certify the absence of defects. CI separately runs the focused regression tests.

## Project boundaries

Apply this standard within each repository's documented responsibility. Do not infer permission to write to hardware or install drivers, treat trusted FFI as a sandbox for untrusted code, or alter source-sample rights and offline-generation constraints. Cross-project integrations must retain versioned contracts and explicit ownership.
