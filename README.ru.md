
<p align="center"><picture><source media="(prefers-color-scheme: dark)" srcset="assets/readme/hero-banner-dark.svg"><source media="(prefers-color-scheme: light)" srcset="assets/readme/hero-banner-light.svg"><img src="assets/readme/hero-banner-light.svg" alt="Процедурная рабочая станция сэмплов VentuSample" width="880"></picture></p>
<h1 align="center">VentuSample — процедурная рабочая станция сэмплов</h1>
<p align="center"><strong>Офлайн-среда C++20 для детерминированной генерации сэмплов, проверенного WAV-импорта, творческой DSP-обработки и экспорта PCM16.</strong></p>
<p align="center"><a href="https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&style=flat-square&label=CI"></a> <a href="LICENSE"><img alt="Apache-2.0" src="https://img.shields.io/github/license/Ivan-VHS/VentuSample?style=flat-square"></a></p>

[Быстрый старт](#быстрый-старт) · [Revival](#обновление-документации-revival) · [Визуальный обзор](#визуальный-обзор) · [Архитектура](#архитектура) · [Документация](docs/README.md)

> **Граница продукта:** VentuSample генерирует и обрабатывает аудио офлайн. Это не real-time audio engine, DAW, VST/AU host или облачный сервис. Необязательная desktop-оболочка Qt 6 отделена от переносимого DSP/core build. Обнаружение plugin manifest само по себе не загружает код.

## Быстрый старт

**Требования:** CMake 3.21+, компилятор C++20 (GCC, Clang или MSVC), Git. Для desktop-приложения нужен Qt 6 Widgets; core можно собрать без Qt. Ninja необязателен.

Сборка desktop в Linux (пакеты Ubuntu 24.04, используемые CI):

~~~sh
sudo apt-get update
sudo apt-get install -y cmake ninja-build g++ qt6-base-dev
git clone https://github.com/Ivan-VHS/VentuSample.git
cd VentuSample
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/VentuSample
~~~

Переносимое ядро без Qt:

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

Windows: MSVC и Qt 6:

~~~sh
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
~~~

## Обновление документации Revival

| Было | Стало |
|---|---|
| Один статичный hero | Парные SVG для светлой и тёмной тем с fallback |
| Генератор, DSP и UI описаны вперемешку | Чёткий офлайн-конвейер и явные границы продукта |
| Снимок и схема не всегда различались | Реальный снимок Studio и отдельно обозначенная схема потока |
| Варианты сборки разбросаны по длинной инструкции | Linux desktop, Windows desktop и core без Qt описаны отдельно |
| Статусы функций распределены по страницам | Сводка с ссылками на DSP, plugin и architecture contracts |

Это обновление документации, **не новая версия ПО и не объявление о релизе**. Поведение и зрелость определяются кодом, тестами, manifest и схемами.

## Реализованный функционал

| Область | Текущая функция | Граница |
|---|---|---|
| Процедурный генератор | Loop, Melody, Drums, Bass и One-Shot Kit; 14 профилей | Повторяемость при одинаковых seed и настройках |
| Audio I/O | PCM16 WAV import/export и версионированный JSON manifest | Проверяемые локальные файлы, без cloud sync |
| Offline DSP | Фильтры, saturation, linked stereo compression, limiting и измерения | Офлайн-буферы, без audio-device callback |
| Quantum Audio Engine | Каталог типизированных модулей и повторно используемые kernels | Творческий DSP-каталог, не физические quantum вычисления |
| Stochastic Router | Необязательная seeded-маршрутизация встроенных processors | Офлайн-обработка, выключена по умолчанию |
| Plugin Manager | Встроенные инструменты и reviewed C++ handler API | Metadata discovery не загружает произвольные DLL/SO |
| MIDI / analysis | Ограниченный MIDI-инспектор и offline FFT | Без MIDI-устройства и playback |
| Reaper bridge | Необязательный C17 integer-expression ABI | Явный локальный source path; не DSP engine |

## Визуальный обзор

<table><tr><td width="50%"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="Снимок рабочего стола VentuSample" width="100%"></a><p><sub><strong>Реальный снимок</strong> — работающий desktop application, не mock-up.</sub></p></td><td width="50%"><img src="assets/readme/workflow.svg" alt="Поток VentuSample: параметры, генератор, аудиобуфер, offline DSP и WAV export" width="100%"><p><sub><strong>Архитектурная иллюстрация</strong> — data path, не скриншот UI.</sub></p></td></tr></table>

## Архитектура

~~~mermaid
flowchart LR
  A[Параметры + seed] --> B[C++20 generator]
  B --> C[Stereo float buffer]
  C --> D[Необязательный offline DSP / routing]
  D --> E[PCM16 WAV]
  D --> F[Versioned JSON manifest]
  G[Qt 6 desktop shell] --> A
  H[Проверенный WAV input] --> D
  I[Native tests] --> B
~~~

## Конфигурация и доверие

Корневой .env, аккаунт, API token и сеть не требуются. Реальные опции CMake:

| Опция | Значение по умолчанию | Назначение |
|---|---:|---|
| VENTU_BUILD_DESKTOP | ON | Сборка desktop target на Qt 6 Widgets, когда Qt доступен |
| VENTU_BUILD_TESTS | ON | Сборка нативных тестов |
| VENTU_ENABLE_SANITIZERS | OFF | ASan/UBSan для поддерживаемых GCC/Clang |
| REAPER_SOURCE_DIR | пусто | Явный локальный checkout Reaper для необязательного expression bridge |

Plugin manifest обнаруживается как metadata. Плагин остаётся неактивным, пока не подключён явно проверенный native C++ handler; произвольные entrypoints и DLL/SO не загружаются.

## Структура репозитория

| Путь | Назначение |
|---|---|
| src/ | C++20 generator, renderer, DSP kernels, plugin manager, Qt UI и bridge |
| src/compat/pytyliki-dsp/ | Перенесённая библиотека обработки C11 |
| integrations/cockos-reaper/ | Необязательные Lua actions для REAPER |
| tests/ | Native DSP, generator, manifest, UI и regression-тесты |
| schemas/ | Версионированные preset, plugin и report contracts |
| docs/ | Audio pipeline, plugins, presets, architecture и UI |
| assets/readme/ | Адаптивный hero и workflow-иллюстрация |
| assets/screenshots/ | Снимки desktop UI |
| .github/workflows/ | Native CI и quality checks |

## Проверки и текущий статус

~~~sh
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
git diff --check
~~~

См. [архитектуру](docs/ARCHITECTURE.md), [матрицу функций](docs/FEATURES.md), [Plugin Manager](docs/PLUGINS.md), [MIDI и resampling](docs/MIDI_AND_RESAMPLING.md), [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md). Скриншоты из CI являются workflow artifacts; passing tests не означают web deployment или tagged Windows release.

## Разработка

См. [CONTRIBUTING.md](CONTRIBUTING.md), [README Visual Standard](docs/README_VISUAL_STANDARD.md) и [SECURITY.md](SECURITY.md). Изменения DSP, ограничений файлов, manifest и plugin lifecycle требуют целевых native tests.

## Лицензия

[Apache-2.0](LICENSE).
