# Neural System & Contract Framework v1 — Ivan-VHS/VentuSample

## Назначение и статус

Этот документ вводит в репозиторий общий каталог из 20 нейросетевых профилей и локальный нейроконтракт. Это **архитектурный и контрактный фундамент**, а не утверждение о наличии обученной нейронной сети.

- Канонический владелец каталога: `Ivan-VHS/DAK`.
- Локальный снимок: `neural-system-catalog.v1.json` (для автономной/offline-проверки).
- Контракт репозитория: `neural-system-contract.v1.json`.
- Схема: `schemas/neural-system-contract.v1.schema.json`.
- Все 20 профилей доступны в каталоге; профильные назначения ниже имеют статус `planned`.
- Исполняемая модель, веса, датасет, обучение и инференс сейчас **не реализованы и отключены**.

Названия в каталоге задают семейства систем, контрактов и инженерных направлений. Они не означают, что в каждом репозитории уже созданы 20 отдельных обученных моделей.

## Роль репозитория

**Роль:** Offline audio-feature and deterministic sample-generation participant.

**Предметная область:** Offline hip-hop sample-pack generation, bounded audio inspection, DSP metadata and reproducible audio assets.

**Технологическая граница:** Existing C++20/Qt 6 workstation and C11 DSP compatibility boundary; offline deterministic processing only.

### Запланированные основные профили

`nscf-04`, `nscf-05`, `nscf-12`, `nscf-14`, `nscf-16`

### Запланированные поддерживающие профили

`nscf-06`, `nscf-07`, `nscf-18`, `nscf-20`

Назначения фиксируют планируемую специализацию проекта, а не текущую функциональную готовность.

## Каталог из 20 профилей

| ID | Интеллектуальная система | Нейроконтракт | Инженерное направление |
|---|---|---|---|
| nscf-01 | Искусственная нейронная сеть | Нейросетевой контракт | Системное проектирование |
| nscf-02 | Нейронная сеть | Нейросетевое соглашение | Системное конструирование |
| nscf-03 | ИНС | Сетевой нейродоговор | Системное программирование |
| nscf-04 | Нейронная модель | Нейронный контракт | Комплексная разработка |
| nscf-05 | Искусственный интеллект | Цифровой нейроконтракт | Архитектурная разработка |
| nscf-06 | Интеллектуальная система | Интеллектуальный контракт | Инженерная разработка |
| nscf-07 | Машинная модель | Когнитивный контракт | Комплексное проектирование |
| nscf-08 | Вычислительная сеть | Искусственно-интеллектуальный контракт | Разработка систем |
| nscf-09 | Когнитивная система | Автономный нейроконтракт | Создание систем |
| nscf-10 | Интеллектуальная модель | Распределённый нейроконтракт | Системная инженерия |
| nscf-11 | Обучаемая система | Децентрализованный нейроконтракт | Архитектурное проектирование |
| nscf-12 | Самообучающаяся модель | Машинный нейроконтракт | Технологическая разработка |
| nscf-13 | Алгоритмическая сеть | Нейросетевой договор | Интеграционная разработка |
| nscf-14 | Цифровой интеллект | Интеллектуальное соглашение | Модульная разработка |
| nscf-15 | Машинный интеллект | Цифровой нейродоговор | Многоуровневая разработка |
| nscf-16 | Генеративная модель | Нейросетевое обязательство | Платформенная разработка |
| nscf-17 | Нейрокомпьютерная система | Сетевой интеллектуальный договор | Инфраструктурная разработка |
| nscf-18 | Программный интеллект | Автономное нейросетевое соглашение | Системное моделирование |
| nscf-19 | Адаптивная модель | Распределённое нейросетевое соглашение | Проектирование архитектуры |
| nscf-20 | Искусственный разум | Программируемый нейроконтракт | Комплексная системная инженерия |

Канонические значения и идентификаторы должны синхронно обновляться во всех пяти репозиториях. DAK является владельцем каталога; локальные копии не должны расходиться с ним.

## Обязанности и границы

- Define future typed audio-feature descriptors and generation metadata without changing the current offline audio contract.
- Preserve seeded generation, reproducible rendering, source provenance and bounded import validation.
- Keep any future model-assisted classification or recommendation advisory and separate from the deterministic DSP renderer.
- Export versioned, inspectable metadata for optional Reaper consumption without mutating WAV assets.

### Явные ограничения

- No real-time audio callback path, DAW/VST/AU hosting or background audio capture is introduced.
- No imported sample is used for training unless its rights, purpose and retention are explicitly approved.
- No model output changes generator or DSP semantics without dedicated deterministic audio regression tests.

## Общий нейроконтракт

### Входной конверт

Каждый будущий запрос обязан включать:

- `contract_id`, `contract_version`, `request_id`, `profile_id`, `purpose`;
- `data_origin`, `payload_schema`, `payload_digest`, `created_at`;
- `rights_reference`, подтверждающий происхождение и допустимость использования данных.

### Выходной конверт

Каждый будущий результат обязан включать:

- идентификатор/версию контракта, `request_id` и `profile_id`;
- `model_id`, `model_version`, `result`, `uncertainty`;
- `evidence_refs`, `input_digest` и временную метку.

Уверенность модели не заменяет доказательства. Неизвестные, отсутствующие и недоступные измерения остаются явно неизвестными, а не преобразуются в ноль или в положительный вывод.

### Ошибки контракта

- `NSCF-E1001-SCHEMA-INVALID` — неверная структура или версия схемы.
- `NSCF-E1002-PROVENANCE-MISSING` — отсутствует происхождение данных.
- `NSCF-E1003-RIGHTS-NOT-VERIFIED` — права/разрешение на использование не подтверждены.
- `NSCF-E1004-PROFILE-UNSUPPORTED` — профиль не поддерживается адаптером.
- `NSCF-E1005-RUNTIME-DISABLED` — runtime не включён.
- `NSCF-E1006-RESOURCE-LIMIT` — превышен установленный лимит ресурсов.
- `NSCF-E1007-REPLAY-MISMATCH` — результат воспроизводимого прогона расходится.

## Предлагаемые межрепозиторные интерфейсы

- `ventusample.audio-feature-evidence.v1` — предоставляет (план): Proposed offline audio feature descriptors with source and processing provenance.
- `ventusample.offline-generation-metadata.v1` — предоставляет (план): Proposed deterministic generation metadata and seed lineage.
- `reaper.neural-typed-ir.v1` — потребляет (план): Future typed feature request/result envelope from Reaper.
- `pytyliki.neural-validation.v1` — потребляет (план): Future deterministic numerical validation adapter.

Все интерфейсы в этом начальном наборе имеют статус `planned`. Они не являются реализованными API и не должны использоваться как действующие зависимости до отдельной реализации, тестирования и совместимого выпуска.

## Безопасность, приватность и управление

- Локальное/offline-исполнение — целевое значение по умолчанию.
- Внешняя передача исходников, пользовательских данных, аудио, телеметрии, устройств и модельных артефактов запрещена по умолчанию.
- Сбор телеметрии отключён; секреты не передаются модели и не попадают в диагностические журналы.
- Обучение и инференс отключены до утверждения задачи, датасета, прав, runtime и бюджетов.
- Результат модели не исполняет системные действия напрямую.
- Изменение модели, датасета, контракта или runtime требует feature-ветки, проверок, человеческого review и плана отката.
- Для DAK/Duck OS любое будущее исполнение модели должно находиться в изолированном пользовательском процессе, не в ядре.

## Обязательные ворота качества

1. Validate the local catalogue and repository contract against the versioned JSON Schema.
2. Record dataset provenance, licence, consent, purpose limitation, retention and deletion rules before training.
3. Separate training, validation and test data by source and prevent leakage across correlated samples.
4. Measure a documented non-neural baseline before claiming model benefit.
5. Test accuracy, calibration, uncertainty, out-of-domain inputs and failure behavior on representative fixtures.
6. Record model identity, training recipe, data snapshot digest, seeds and build/runtime versions for reproducibility.
7. Enforce explicit memory, CPU/GPU, queue, timeout and latency budgets before enabling execution.
8. Threat-model untrusted inputs, imported model artifacts, deserialization and model-output side effects.
9. Require human review, a feature branch, CI evidence and a rollback plan for model or contract changes.
10. Keep local/offline execution as the default and verify that no data leaves the repository or host without explicit approval.

До прохождения соответствующих ворот нельзя помечать модель, обучение, инференс или интеграцию как `implemented`.

## Состояние реализации

| Компонент | Состояние |
|---|---|
| Каталог 20 профилей | Специфицирован |
| Контракт репозитория | Специфицирован |
| JSON Schema | Добавлена в этом наборе изменений; CI-проверка ещё не подтверждена |
| Модельная архитектура под конкретную задачу | Не выбрана |
| Датасет и права на данные | Не определены |
| Обучение | Отключено / не реализовано |
| Инференс/runtime | Отключён / не реализован |
| Производственные метрики | Не измерялись |

Этот документ не заявляет о существовании модели, весов, результатов обучения или эксплуатационных характеристик.
