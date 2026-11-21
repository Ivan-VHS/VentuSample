# Improvement, dissonance, and conflict resolution

## Purpose and scope

This contract turns the supplied 20 triplets into a repeatable engineering improvement process for **VentuSample**, an offline C++20/Qt workstation for deterministic sample generation and versioned WAV pack export.

It complements the existing audio-quality and release-readiness checks. It does not replace DSP tests, listening review, source-rights review, code review, or CI.

## The 20-control vocabulary

| # | Improvement outcome | Dissonance signal | Conflict pattern | Acceptance intent |
|---:|---|---|---|---|
| 1 | Совершенствование | Дисгармония | Конфликты | Align audio acceptance criteria and rendered output. |
| 2 | Преображение | Разлад | Противоречия | Restore reliability when rendering, cancellation, or export state is inconsistent. |
| 3 | Оптимизация | Несозвучие | Столкновения | Close unsafe or ambiguous audio-input and filesystem paths. |
| 4 | Модернизация | Дисбаланс | Несовместимости | Modernize audio profiles and pack contracts without breaking supported consumers. |
| 5 | Улучшение | Диспропорция | Разногласия | Prioritize audible and workflow improvements instead of accumulating unused controls. |
| 6 | Развитие | Несогласованность | Расхождения | Align generator, desktop UI, DSP, and export around shared parameter ownership. |
| 7 | Преимущественное развитие | Разноречие | Коллизии | Direct effort toward the generator's primary sample-pack outcomes. |
| 8 | Усовершенствование | Противоречивость | Споры | Make disputes over musical behavior and control semantics reproducible. |
| 9 | Реконструкция | Какофония | Несогласованности | Rebuild fragile render or export paths at a stable module boundary. |
| 10 | Рефакторинг | Дискретность | Противоборства | Refactor code for clarity while preserving audio and file compatibility. |
| 11 | Переосмысление | Разобщённость | Соперничества | Re-evaluate old assumptions when sound-design profiles or user workflows change. |
| 12 | Обновление | Неоднородность | Раздвоения | Keep profile definitions, schemas, README claims, and manifest versions synchronized. |
| 13 | Преобразование | Несоответствие | Пересечения | Transform imported or rendered audio without losing timing, musical metadata, or provenance. |
| 14 | Обогащение | Расхождение | Накладки | Combine complementary profiles and generation stages without hidden coupling. |
| 15 | Усиление | Разноголосица | Неувязки | Strengthen weak points revealed by repeated audio or export failures. |
| 16 | Доработка | Дисгармоничность | Диссонансы | Finish resource, cancellation, file, and empty-state edge cases before release. |
| 17 | Возвышение | Конфликтность | Антагонизмы | Improve desktop clarity and accessibility without changing sound semantics. |
| 18 | Прогрессирование | Неслаженность | Несоответствия | Tune audio quality and performance using measured evidence. |
| 19 | Эволюционирование | Раздвоенность | Разобщённости | Evolve profile and export formats incrementally while retaining compatible packs. |
| 20 | Комплексное совершенствование | Поляризация | Раздоры | Review emergent issues across profiles, rendering, desktop, DSP, export, rights, and documentation. |

The first column names the intended positive change. The second names a symptom that warrants investigation. The third names a possible conflict class. These terms are diagnostic vocabulary, not findings: a word in the registry does not prove that a defect exists.

## Resolution lifecycle

1. **Detect:** record the affected profile, parameter, render seed, audio input, environment, expected result, actual result, and a reproducible fixture.
2. **Reproduce:** reduce the report to a deterministic render, WAV parser, UI/engine integration, or manifest test.
3. **Confirm the root cause:** distinguish a DSP defect from a profile-specific choice, source-audio limitation, export mismatch, or unsupported format.
4. **Remediate:** make the smallest coherent change that preserves deterministic behavior, resource bounds, audio integrity, and supported pack compatibility.
5. **Verify:** add regression coverage, run native/Qt builds and audio/manifest tests, and compare documentation with actual behavior.
6. **Promote:** use a reviewable branch and pull request. Do not force-push over protected work, bypass required checks, or silently merge unresolved conflicts.

## Priority and acceptance

- **P0:** unsafe filesystem behavior, unbounded allocation, malformed output, or data loss; block release until contained.
- **P1:** incorrect audio, corrupted packs, unstable deterministic output, or broken manifest contracts; fix before promotion.
- **P2:** platform build, UI/renderer integration, profile parity, or repeatability regressions; fix or document an evidence-backed deferral.
- **P3:** clarity, maintainability, and non-blocking usability improvements; schedule with an owner and acceptance criteria.

A control may move to `detected`, `root_cause_confirmed`, `remediated`, `verified`, or `deferred`. Any assessed state must include repository-relative evidence. `verified` requires evidence that exists in the repository. A deferral requires a concise reason in the profile's `note` field. Never label an item verified solely because a branch or PR exists.

## Project-specific non-negotiables

- Rendering remains offline and deterministic for a fixed seed and parameters.
- PCM/WAV parsing stays bounded; imported audio is untrusted input and malformed files must be rejected safely.
- Key/mode labels describe selected generator settings unless a feature explicitly performs source analysis; imported source rights are not inferred.
- Existing pack manifests remain versioned and backward compatibility is tested before promotion.
- The application does not certify copyright ownership or grant a commercial license for imported material.

## Validation

The versioned profile is `improvement-conflict.v1.json`; its structure is described by `schemas/improvement-conflict.v1.schema.json`.

```bash
python -m unittest discover -s tests -p test_improvement_conflict.py -v
python tools/improvement_conflict.py
python tools/improvement_conflict.py --format json
```

The validator checks the exact 20 triplets, identifiers, required remediation and verification fields, lifecycle states, and repository-relative evidence paths. It validates the governance register; it is **not** a static source-code conflict scanner and does not claim that unassessed controls have been resolved.
