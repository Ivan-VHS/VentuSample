# VentuSample — Quality, Cohesion and Commercial Standard

**Profile:** Offline procedural hip-hop sample-pack workstation  
**Status:** normative product-engineering and release-control standard  
**Companion manifest:** [`product-readiness.json`](../product-readiness.json)  
**Validator:** `python scripts/validate_product_readiness.py`

## Purpose and interpretation

VentuSample создаёт локальные процедурные музыкальные лупы и наборы one-shot с повторяемыми параметрами, WAV-экспортом и версионированным манифестом. Его коммерческая ценность — быстрый воспроизводимый стартовый материал для музыкального производства, а не DAW, плагин или маркетплейс.

The standard has three equal and separately visible axes:

- **Quality** means correct, bounded, secure, testable and evidence-backed behavior.
- **Cohesion** means one understandable end-to-end product contract, stable module ownership and compatible interfaces. It does **not** mean collapsing the codebase into a single monolith.
- **Commercial readiness** means a credible value proposition, discoverable onboarding, lawful distribution, explicit support boundaries and honest market claims. It does **not** imply that pricing, revenue, customer demand or profitability have been established.

| # | Качество | Цельность | Коммерческий вектор | Проверяемое требование |
|---:|---|---|---|---|
| 01 | Качественность | Качество | Коммерческий | У каждого обещания есть критерий приёмки, реализация и проверяемое подтверждение. |
| 02 | Добротность | Добротность | Торговый | Поставка описывает точный состав продукта, формат, платформы и ограничения. |
| 03 | Безупречность | Безупречность | Предпринимательский | Каждое изменение проходит владельца, ревью, тестирование и контролируемый выпуск. |
| 04 | Совершенство | Совершенство | Рыночный | Позиционирование основано на реальной потребности и не выдаёт гипотезы за исследование рынка. |
| 05 | Цельность | Мастерство | Деловой | Основной пользовательский сценарий прослеживается от входа до результата. |
| 06 | Целостность | Профессионализм | Финансовый | Стоимость владения, зависимости и требования к окружению описаны без неподтверждённых обещаний. |
| 07 | Монолитность | Надёжность | Прибыльный | Сквозной путь устойчив, ошибки локализованы; монолитность означает согласованность, а не один гигантский модуль. |
| 08 | Согласованность | Основательность | Доходный | Источник правды для функций, версий, метаданных и пользовательских обещаний однозначен. |
| 09 | Последовательность | Гармоничность | Экономический | Команды, настройки, форматы и ошибки ведут себя предсказуемо и не создают лишних операций. |
| 10 | Системность | Согласованность | Хозяйственный | Межмодульные сценарии покрыты интеграционными проверками и имеют ясные границы ответственности. |
| 11 | Структурированность | Цельность | Бизнесовый | Модули сгруппированы вокруг предметных задач, а не случайных технических деталей. |
| 12 | Упорядоченность | Целостность | Маркетинговый | Публичные описания, примеры, скриншоты и фактический интерфейс не противоречат друг другу. |
| 13 | Непротиворечивость | Монолитность | Клиентский | Условия, ограничения, сообщения об ошибках и результат понятны без скрытых допущений. |
| 14 | Однородность | Нераздельность | Сбытовой | Установка, запуск, экспорт и обновление образуют один проверенный путь поставки. |
| 15 | Унификация | Единство | Промышленный | Повторяющиеся алгоритмы и контракты общие; доменная логика остаётся в профильных модулях. |
| 16 | Стандартизация | Слаженность | Корпоративный | Версии, лицензия, безопасность, конфиденциальность и совместимость документируются единообразно. |
| 17 | Полнота | Ритмичность | Инвестиционный | Выпуски опираются на измеримые этапы, журнал изменений и сохранение обратной совместимости. |
| 18 | Точность | Размеренность | Монетизируемый | Ресурсы, производительность и потенциальные платные границы измеримы; платная функция не заявляется до реализации. |
| 19 | Выверенность | Синхронность | Рентабельный | Код, тесты, схема, документация и пакеты обновляются в одном изменении. |
| 20 | Профессионализм | Упорядоченность | Капитализированный | Лицензия, права на материалы, происхождение артефактов и правила распространения определены до коммерческого использования. |

## Normative implementation rules

1. Every capability described as implemented MUST point to working source, a test, a shipped schema or an observable user workflow. A design document alone is not proof of runtime behavior.
2. Every partial capability MUST state the missing boundary. Planned work MUST NOT appear as shipped functionality or be used in a sales claim.
3. Each user journey MUST define accepted inputs, output artifacts, failure behavior, resource limits and recovery or cancellation behavior.
4. Each public interface MUST have an owner, stable identifier, version, compatibility rule and validation path. Cross-project adapters MUST remain optional and explicit.
5. Each release MUST identify its source revision, supported platforms, build prerequisites, license, changes, known limitations and verification result.
6. Imported files and external metadata MUST be treated as untrusted. Validation MUST precede allocation, execution, loading, extraction or state changes.
7. Product and marketing copy MUST separate measured facts, tested behavior, approximations, roadmap proposals and unsupported claims.
8. Pricing, paid support, return on investment, market share and customer adoption MUST remain unpublished or explicitly labelled unverified until supported by real evidence and an approved business decision.
9. Privacy, telemetry, native loading, device access and export MUST follow the project's existing consent and safety boundaries. Commercial pressure MUST NOT weaken those controls.
10. Documentation, schemas, examples, user interface text and release artifacts MUST be updated together when observable behavior or compatibility changes.

## Product-readiness passport

The adjacent `product-readiness.json` is the machine-readable current-state passport. It records five controls for each axis. Each control has a stable ID, a status, an acceptance statement and repository-local evidence.

Allowed statuses:

- `implemented` — the control is present and the evidence path exists.
- `partial` — a bounded part is present; the statement must identify the remaining limitation.
- `planned` — not yet shipped; it contributes no evidence coverage.
- `not_applicable` — excluded from the denominator with an explicit reason.

Run the validator from the repository root:

```bash
python scripts/validate_product_readiness.py
python scripts/validate_product_readiness.py --json
python scripts/validate_product_readiness.py --self-test
```

The validator rejects missing evidence files, duplicate control IDs, unsupported statuses, malformed project metadata and incomplete axes. Its coverage figures describe **documented evidence coverage only**. They are not a security certification, product-quality rating, market-fit result, financial forecast or guarantee of commercial success.

## Commercial policy

The current public position is community/open-source distribution under the repository's checked-in license. No price, paid tier, paid support SLA, guaranteed return, customer count or market-size claim is created by this standard. Any future commercial offer requires a separate decision covering:

- permitted use and redistribution under the actual license;
- deliverables, supported platforms, version policy and installation route;
- support scope, response commitments and escalation;
- privacy, update provenance, artifact integrity and security response;
- operating costs, dependency obligations and measurable user value.

Until that decision is made, the repository remains an openly distributed engineering product with no implied paid commitment.

## Release acceptance checklist

- [ ] Product passport validates and all evidence paths resolve.
- [ ] Automated tests and the supported build complete on the declared CI matrix.
- [ ] Public claims match current implementation status.
- [ ] Installation, first-run and export paths are reproducible.
- [ ] License and third-party material obligations are clear.
- [ ] Versioned interfaces and compatibility notes are current.
- [ ] Privacy, safety and privilege boundaries are preserved.
- [ ] Known gaps and non-goals are visible before distribution.
