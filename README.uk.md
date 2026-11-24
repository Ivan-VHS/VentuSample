
<p align="center"><picture><source media="(prefers-color-scheme: dark)" srcset="assets/readme/hero-banner-dark.svg"><source media="(prefers-color-scheme: light)" srcset="assets/readme/hero-banner-light.svg"><img src="assets/readme/hero-banner-light.svg" alt="Процедурна робоча станція семплів VentuSample" width="880"></picture></p>
<h1 align="center">VentuSample — процедурна робоча станція семплів</h1>
<p align="center"><strong>Офлайн-середовище C++20 для детермінованої генерації семплів, перевіреного імпорту WAV, творчої DSP-обробки та експорту PCM16.</strong></p>
<p align="center"><a href="https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&style=flat-square&label=CI"></a> <a href="LICENSE"><img alt="Apache-2.0" src="https://img.shields.io/github/license/Ivan-VHS/VentuSample?style=flat-square"></a></p>

[Швидкий старт](#швидкий-старт) · [Revival](#оновлення-документації-revival) · [Візуальний огляд](#візуальний-огляд) · [Архітектура](#архітектура) · [Документація](docs/README.md)

> **Межа продукту:** VentuSample генерує та обробляє аудіо офлайн. Це не real-time audio engine, DAW, VST/AU host чи хмарний сервіс. Необов’язкова desktop-оболонка Qt 6 відокремлена від переносимого DSP/core build. Виявлення plugin manifest саме по собі не завантажує код.

## Швидкий старт

**Вимоги:** CMake 3.21+, компілятор C++20 (GCC, Clang або MSVC) і Git. Для desktop-застосунку потрібен Qt 6 Widgets; core можна зібрати без Qt. Ninja необов’язковий.

Linux desktop build (пакети Ubuntu 24.04, що використовуються CI):

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

Переносиме ядро без Qt:

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

Windows: MSVC і Qt 6:

~~~sh
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
~~~

## Оновлення документації Revival

| Було | Стало |
|---|---|
| Один статичний hero | Парні SVG для світлої й темної тем з fallback |
| Генератор, DSP та UI описані разом | Чіткий офлайн-конвеєр і межі продукту |
| Знімок і схема не завжди розрізнялися | Реальний знімок Studio та окремо позначена схема потоку |
| Варіанти збирання розкидані | Linux desktop, Windows desktop і core без Qt описані окремо |
| Статуси функцій розподілені між сторінками | Зведення з посиланнями на DSP, plugin та architecture contracts |

Це оновлення документації, **не нова версія ПЗ й не оголошення релізу**. Поведінку визначають код, тести, manifest і схеми.

## Реалізований функціонал

| Напрям | Поточна поведінка | Межа |
|---|---|---|
| Процедурний генератор | Loop, Melody, Drums, Bass і One-Shot Kit; 14 профілів | Повторюваність за однакових seed і налаштувань |
| Audio I/O | PCM16 WAV import/export і версіонований JSON manifest | Перевірені локальні файли, без cloud sync |
| Offline DSP | Фільтри, saturation, linked stereo compression, limiting і вимірювання | Офлайн-буфери, без audio-device callback |
| Quantum Audio Engine | Каталог типізованих модулів і повторно використовувані kernels | Творчий DSP-каталог, не фізичні quantum обчислення |
| Stochastic Router | Необов’язкова seeded-маршрутизація вбудованих processors | Офлайн-обробка, вимкнена за замовчуванням |
| Plugin Manager | Вбудовані інструменти та reviewed C++ handler API | Metadata discovery не завантажує довільні DLL/SO |
| MIDI / analysis | Обмежений MIDI-інспектор і offline FFT | Без MIDI-пристрою та playback |
| Reaper bridge | Необов’язковий C17 integer-expression ABI | Явний локальний source path; не DSP engine |

## Візуальний огляд

<table><tr><td width="50%"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="Знімок desktop-застосунку VentuSample" width="100%"></a><p><sub><strong>Реальний знімок</strong> — запущений застосунок, не mock-up.</sub></p></td><td width="50%"><img src="assets/readme/workflow.svg" alt="Потік VentuSample: параметри, генератор, аудіобуфер, offline DSP та WAV export" width="100%"><p><sub><strong>Архітектурна ілюстрація</strong> — data path, а не скриншот UI.</sub></p></td></tr></table>

## Архітектура

~~~mermaid
flowchart LR
  A[Параметри + seed] --> B[C++20 generator]
  B --> C[Stereo float buffer]
  C --> D[Необов’язковий offline DSP / routing]
  D --> E[PCM16 WAV]
  D --> F[Versioned JSON manifest]
  G[Qt 6 desktop shell] --> A
  H[Перевірений WAV input] --> D
  I[Native tests] --> B
~~~

## Конфігурація та довіра

Кореневі .env, обліковий запис, API token і мережа не потрібні. Реальні опції CMake:

| Опція | Типове значення | Призначення |
|---|---:|---|
| VENTU_BUILD_DESKTOP | ON | Збирання desktop target із Qt 6 Widgets, коли Qt доступний |
| VENTU_BUILD_TESTS | ON | Збирання нативних тестів |
| VENTU_ENABLE_SANITIZERS | OFF | ASan/UBSan для підтримуваних GCC/Clang |
| REAPER_SOURCE_DIR | порожньо | Явний локальний checkout Reaper для необов’язкового expression bridge |

Plugin manifest виявляється як metadata. Плагін залишається неактивним, доки не підключено явно перевірений native C++ handler; довільні entrypoints і DLL/SO не завантажуються.

## Структура репозиторію

| Шлях | Призначення |
|---|---|
| src/ | C++20 generator, renderer, DSP kernels, plugin manager, Qt UI та bridge |
| src/compat/pytyliki-dsp/ | Перенесена бібліотека обробки C11 |
| integrations/cockos-reaper/ | Необов’язкові Lua actions для REAPER |
| tests/ | Native DSP, generator, manifest, UI і regression-тести |
| schemas/ | Версіоновані preset, plugin і report contracts |
| docs/ | Audio pipeline, plugins, presets, architecture та UI |
| assets/readme/ | Адаптивний hero і workflow-ілюстрація |
| assets/screenshots/ | Знімки desktop UI |
| .github/workflows/ | Native CI та quality checks |

## Перевірки та поточний статус

~~~sh
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
git diff --check
~~~

Див. [архітектуру](docs/ARCHITECTURE.md), [матрицю функцій](docs/FEATURES.md), [Plugin Manager](docs/PLUGINS.md), [MIDI і resampling](docs/MIDI_AND_RESAMPLING.md), [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md). CI screenshots є workflow artifacts; passing tests не означають web deployment чи tagged Windows release.

## Розробка

Читайте [CONTRIBUTING.md](CONTRIBUTING.md), [README Visual Standard](docs/README_VISUAL_STANDARD.md) і [SECURITY.md](SECURITY.md). Зміни DSP, file bounds, manifest або plugin lifecycle потребують цільових native tests.

## Ліцензія

[Apache-2.0](LICENSE).
