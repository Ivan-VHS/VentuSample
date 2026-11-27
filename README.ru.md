# VentuSample — офлайн-рабочая станция процедурного аудио

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

[![CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Release workflow](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/release.yml?branch=main&label=Release)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/release.yml) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)

> Детерминированная генерация сэмплов на C++20 и offline DSP с ограниченным WAV I/O, воспроизводимой маршрутизацией и версионированными manifest наборов.

**Сведения о проекте:** разработка · версия 0.2.0 · ядро C++20 и DSP compatibility layer на C11 · необязательный desktop Qt 6 Widgets · offline-рендеринг · Apache-2.0 · **Документация проверена:** 2026-11-27 UTC

<p align="center"><img src="assets/screenshots/ventusample-studio.png" alt="Офлайн-рабочая станция VentuSample" width="100%"></p>

## Назначение и архитектура

VentuSample создаёт воспроизводимые наборы сэмплов и локально обрабатывает аудиофайлы. Ядро C++20 отвечает за проверку параметров, генерацию паттернов с фиксированным seed, стереообработку, offline DSP-маршрутизацию, импорт/экспорт WAV и формирование manifest. Совместимый слой C11 предоставляет низкоуровневые DSP- и MIDI/file-примитивы. Необязательное приложение Qt 6 Widgets предоставляет desktop-интерфейс, но не является аудиохостом реального времени.

~~~mermaid
flowchart TB
  U["Профиль / seed / параметры"] --> V["Проверка и ресурсные лимиты"]
  V --> G["Планировщик паттернов и renderer"]
  G --> F["Стерео float processing"]
  F --> D["Offline DSP и stochastic routing"]
  D --> M["Master / RMS и peak guard"]
  M --> W["Ограниченные PCM16 WAV assets"]
  V --> J["Версионированный pack manifest v1/v2"]
  W --> P["Локальный sample pack"]
  J --> P
  Q["Необязательный Qt 6 desktop"] --> U
~~~

## Возможности

- **Процедурная генерация:** детерминированные стереопаттерны с seed по 14 документированным hip-hop и смежным профилям, включая редактируемый профиль Billionaire Golden Era.
- **Golden-era crate workflow:** детерминированная пересборка четырёх срезов в порядке 0–2–1–3, взвешенный подбор kick/snare, сдержанные hats, общий swing и воспроизводимые переходы.
- **Импорт и нарезка WAV:** ограниченный PCM16 mono/stereo input (8–192 kHz, до 128 MiB и 20 млн кадров), размещение четырёх срезов и ресэмплинг по полутонам в диапазоне ±12. Изменение высоты тона меняет и скорость; time-stretch с сохранением темпа не реализован.
- **Offline DSP:** фильтры, saturation, обработка связанной стереошины, RMS matching, защита sample peak и ограниченная маршрутизация эффектов.
- **Stochastic router:** восемь взвешенных эффектов в serial, parallel и hybrid-графах; триггеры file/bar/generator-onset; детерминированная маршрутизация по seed/variation; wet ≤65%, feedback ≤72%, fades 5–20 ms, ограниченная RMS-компенсация и sample-peak-защита −0,3 dBFS. Интегрирован в offline WAV export и представлен как 19-й встроенный плагин.
- **Анализ:** peak, RMS и crest factor для файлов; offline FFT peak, centroid и rolloff на 85%. Доступ к аудиоустройствам не требуется.
- **QMR-20:** 20 параметров качества, cohesion и ритмики с regression gates; это детерминированные инженерные проверки, а не универсальная или субъективная оценка звучания.
- **Тональные метаданные:** key/mode указываются только для тонального материала; ударным без определённой высоты не назначается вымышленная тональность.
- **Обмен наборами:** manifest `ventusample.pack.v1` и `ventusample.pack.v2` с WAV assets. Текущий потребитель Reaper проверяет и сравнивает метаданные manifest v1, не анализируя и не изменяя waveform.

Профили — оригинальные процедурные паттерны, а не записи и не заявления о воспроизведении конкретного артиста. За импортированное аудио отвечает пользователь; программа не подтверждает авторские права или лицензионную чистоту материала.

## Сборка и проверка

Требуются CMake 3.21+, компилятор C++20 и компилятор C для DSP-слоя C11. Qt 6 Widgets необязателен. Переносимое ядро и тесты можно собирать без Qt.

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

Переносимое ядро без Qt:

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

Desktop-цель создаётся только при обнаружении Qt 6 Widgets. Без SDK переносимое ядро и доступные тесты остаются собираемыми.

## Границы аудио и файлов

| Поверхность | Контракт | Граница |
|---|---|---|
| Presets | `ventusample.preset.v1` | Проверенные локальные настройки |
| Pack exchange | `ventusample.pack.v1 / v2` | JSON manifest и WAV assets |
| Audio output | RIFF PCM16 stereo WAV | Ограниченный локальный render с peak/RMS metadata |
| DSP graph | Native offline DSP | Без real-time VST/AU hosting |
| Analysis | Offline FFT и метрики файлов | Локальные данные; без захвата с аудиоустройства |

Рендеринг выполняется offline. Сейчас проект не предоставляет real-time playback, аудиодвижок с доступом к устройствам, VST/AU-хост, time-stretch с сохранением темпа или GPU dispatch. Не заявляются фиксированная задержка real-time, объём RAM или универсальная производительность для всех sample rate.

## Тесты и доказательства производительности

CTest проверяет детерминированную генерацию, WAV boundaries, ресэмплинг, MIDI-файлы, offline spectrum analysis, маршрутизацию плагинов и runtime-транзакции. UI-тесты Qt регистрируются только при наличии Qt 6 Widgets. Проценты coverage или fuzzing не заявляются без опубликованных отчётов.

Тесты подтверждают детерминированное и ограниченное поведение, а не задержку воспроизведения в реальном времени или универсальный рейтинг оборудования.

## Интеграции и документация

Обмен с Reaper использует файлы и версионированные форматы. Текущий Reaper consumer читает метаданные manifest v1 и оставляет WAV assets нетронутыми. Необязательная интеграция нативных выражений Reaper включается только при явной передаче локального дерева исходников Reaper в CMake. Прямой адаптер нативных метрик/DSP Pytyliki пока запланирован.

- [Матрица функций](docs/FEATURES.md) · [Архитектура](docs/ARCHITECTURE.md)
- [CRATE feature details](docs/FEATURES.md) · [Hip-hop preset examples](examples/compare_packs.py)
- [Quality Monolith Rhythm](docs/QUALITY_MONOLITH_RHYTHM.md) · [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)
- [Плагины и lifecycle](docs/PLUGINS.md) · [MIDI и ресэмплинг](docs/MIDI_AND_RESAMPLING.md)
- [Интеграция исходников и лицензии](docs/SOURCE_INTEGRATION.md) · [Индекс документации](docs/README.md)
- [Контроль документации](docs/DOCUMENTATION_CONTROL.md)

## Безопасность, участие и лицензия

Считайте импортированные WAV и preset packs недоверенными данными. Перед обработкой проверяйте размер файла, RIFF chunks, число кадров и заявленные форматы. Внешние native-библиотеки не загружаются неявно. Управление драйверами, прошивкой и системным звуком не выполняется.

Перед изменением DSP-поведения, форматов файлов или plugin-контрактов прочитайте [CONTRIBUTING.md](CONTRIBUTING.md). Лицензия проекта — [Apache-2.0](LICENSE); сохраняйте уведомления сторонних компонентов.
