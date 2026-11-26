# VentuSample — автономная рабочая станция процедурного аудио

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

> Детерминированная рабочая станция на C++20 для генерации сэмплов и offline DSP с ограниченным импортом/экспортом PCM16 WAV, версионируемыми pack-manifest и необязательной оболочкой Qt 6.

**Спецификация**

| Поле | Текущее состояние |
|---|---|
| Репозиторий | [Ivan-VHS/VentuSample](https://github.com/Ivan-VHS/VentuSample) |
| Версия проекта | 0.2.0 (контракт проекта и CMake) |
| Статус | Разработка; offline-аудиоядро |
| Ядро | C++20; C11 DSP compatibility layer |
| Desktop | Необязательная оболочка Qt 6 Widgets |
| Сборка | CMake 3.21+ |
| Аудиовывод | PCM16 WAV, детерминированный локальный рендеринг |
| Pack-форматы | Manifest v1 и v2 |
| Лицензия | [Apache-2.0](LICENSE) |
| Кодекс поведения | Отдельный `CODE_OF_CONDUCT.md` не опубликован |
| Проверка документации | 2026-11-26 |

<p align="center"><img src="assets/screenshots/ventusample-studio.png" alt="Рабочая станция VentuSample" width="100%"></p>

## Содержание
- [Назначение и архитектура](#назначение-и-архитектура)
- [Сборка и проверка](#сборка-и-проверка)
- [Аудио- и файловые контракты](#аудио-и-файловые-контракты)
- [DSP и система плагинов](#dsp-и-система-плагинов)
- [Интеграции](#интеграции)
- [Ограничения и права](#ограничения-и-права)
- [Документация и версии](#документация-и-версии)

## Назначение и архитектура

VentuSample генерирует воспроизводимые sample packs и локально обрабатывает аудиофайлы. Ядро C++20 отвечает за валидацию параметров, детерминированную генерацию паттернов, обработку stereo float, offline DSP-маршрутизацию, WAV I/O и создание manifest. Desktop-цель Qt 6 Widgets необязательна и собирается только при наличии SDK.

~~~mermaid
flowchart TB
  U["Параметры / seed / variation"] --> V["Проверка и ресурсные ограничения"]
  V --> G["Планировщик паттернов и рендерер"]
  G --> F["Stereo float buffer"]
  F --> D["Offline DSP / stochastic router"]
  D --> M["Master и peak guard"]
  M --> W["PCM16 WAV assets"]
  V --> J["Версионируемый pack manifest v1/v2"]
  W --> P["Каталог пакета"]
  J --> P
  Q["Необязательный Qt 6 desktop"] --> U
~~~

Рендеринг выполняется offline в рабочем потоке. Real-time audio callback и доступ к аудиоустройству не предоставляются.

## Сборка и проверка

Требуются CMake 3.21+ и компилятор C++20. Для desktop-приложения установите Qt 6 Widgets.

### Ядро и доступные тесты

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

Без Qt 6 явно соберите ядро и его тесты:

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

UI-тесты Qt доступны только при конфигурации Qt Widgets target. Пропуск desktop-target не означает успешную сборку desktop-приложения.

## Аудио- и файловые контракты

| Поверхность | Контракт |
|---|---|
| Созданные assets | RIFF/WAVE PCM16 |
| Импорт | PCM16 mono/stereo WAV |
| Частота источника | 8–192 кГц |
| Ограничения источника | не более 128 МиБ и 20 000 000 frames |
| Chop | Импорт четырёх срезов и размещение по beat |
| Pitch shift | ±12 полутонов методом resampling; длительность/скорость меняются |
| Частоты экспорта | 22,05; 44,1; 48 или 96 кГц |
| Pack manifest | v1 и v2; v2 добавляет необязательные тональные и routing-метаданные |
| Reaper audit | Читает метаданные manifest; WAV остаются нетронутыми |

Импортируемое аудио считается недоверенным вводом и проходит проверку формата, чанков, числа кадров и размера. Тональные метки описывают настройки генератора, а не автоматическое определение тональности импортированного материала. Ударным assets не присваивается вымышленная тональность.

## DSP и система плагинов

Текущий проект включает seed-based процедурные профили, общий swing/timing schedule, Golden Era arrangement, ограниченный stereo mastering, восьмиэффектный stochastic offline router и каталог Quantum Audio Engine с 133 именованными module identity на основе 21 переиспользуемого DSP kernel.

Stochastic router обрабатывает уже отрендеренный аудиобуфер перед кодированием WAV. Onset-trigger использует event frames от рендерера; он не обнаруживает транзиенты в произвольном импортированном аудио. Quantum Audio Engine — offline DSP-каталог и цепочка обработки, а не физический квантовый компьютер и не real-time DAW engine.

См. [Features](docs/FEATURES.md), [Architecture](docs/ARCHITECTURE.md), [Plugins](docs/PLUGINS.md), [Quality Monolith Rhythm](docs/QUALITY_MONOLITH_RHYTHM.md) и [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md).

## Интеграции

VentuSample экспортирует manifest v1/v2 и WAV assets. Текущий Reaper consumer проверяет и сравнивает метаданные pack; он не анализирует, не переписывает и не рендерит waveform WAV. Прямой native metrics adapter Pytyliki запланирован и должен быть opt-in, пройти ABI differential tests до объявления доступным.

Мост Reaper в CMake отключён, пока явно не задан локальный каталог исходников Reaper. Внешняя native-библиотека автоматически не загружается.

## Ограничения и права

> VentuSample — offline-рабочая станция, а не real-time audio engine и не DAW plugin host.

Real-time playback, захват аудиоустройства, DAW plugin hosting, tempo-preserving time-stretch и GPU dispatch не реализованы. Транспонирование источника использует resampling, поэтому изменение высоты тона также меняет длительность воспроизведения. Рендерер использует portable scalar path; универсальные показатели latency или throughput не заявляются.

Пользователи отвечают за права на импортированные сэмплы, созданные outputs и распространяемые packs. Репозиторий не заявляет право собственности или лицензионную чистоту стороннего аудиоматериала. Не добавляйте приватные пути к samples или нелицензированные записи в публичные тестовые данные.

## Документация и версии

Начните с [индекса документации](docs/README.md) и [информационной архитектуры](docs/INFORMATION_ARCHITECTURE.md).

- [Матрица функций](docs/FEATURES.md)
- [Архитектура и аудиоконтракт](docs/ARCHITECTURE.md)
- [Руководство пользователя](docs/USER_GUIDE_RU.md)
- [Manifest и Reaper pack audit](docs/REAPER_PACK_AUDIT.md)
- [Plugin API и lifecycle](docs/PLUGINS.md)
- [MIDI и resampling](docs/MIDI_AND_RESAMPLING.md)
- [Quality Monolith Rhythm](docs/QUALITY_MONOLITH_RHYTHM.md)
- [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)
- [Контроль документации](docs/DOCUMENTATION_CONTROL.md)

Контракт проекта и CMake объявляют версию 0.2.0; это само по себе не подтверждает стабильный релиз. Изменения публичного pack-формата требуют обновления схем, тестов совместимости и примечаний о миграции. Перед изменением DSP-input, плагинов или pack-контрактов прочитайте [CONTRIBUTING.md](CONTRIBUTING.md) и [SECURITY.md](SECURITY.md). Лицензия проекта — [Apache-2.0](LICENSE); сохраняйте уведомления сторонних компонентов.