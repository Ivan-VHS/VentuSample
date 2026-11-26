# 🎛️ VentuSample — рабочая станция процедурного аудио

<p align="center">[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)</p>

[![CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Release](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/release.yml?branch=main&label=Release)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/release.yml) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)

> **Детерминированная генерация сэмплов и offline DSP. Без имитации real-time-движка.**

**Состояние:** Разработка · офлайн-аудиоядро реализовано · Qt desktop частично · **Версия:** 0.2.0 · **License:** Apache-2.0 · **Стандарт README:** Psychoporter 666 v1.0 · **Дата проверки:** 2026-11-25 UTC · **Source:** [Ivan-VHS/VentuSample](https://github.com/Ivan-VHS/VentuSample)

<p align="center"><img src="assets/readme/dashboard.svg" alt="VentuSample project overview" width="100%"></p>
<p align="center"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample desktop application screenshot" width="100%"></a></p>

## Быстрая навигация

[🚀 Demo: Studio screenshot](assets/screenshots/ventusample-studio.png) · [Documentation](docs/README.md) · [Source](https://github.com/Ivan-VHS/VentuSample) · [Issues](https://github.com/Ivan-VHS/VentuSample/issues)

## Назначение и границы

Нативная рабочая станция процедурных сэмплов с ядром C++20, совместимым DSP-слоем C11 и необязательным desktop-приложением Qt 6 Widgets. Рендеринг выполняется offline. Это не real-time DAW, VST/AU-хост, облачный сервис или аудиодрайвер.

## Возможности

- Детерминированная процедурная генерация стерео по 14 документированным hip-hop и смежным профилям.
- Пересборка golden-era sample crates, взвешенный подбор барабанных тембров и воспроизводимые вариации.
- Ограниченный импорт/экспорт PCM16 WAV, размещение четырёх срезов и транспонирование по полутонам.
- Offline DSP-маршрутизация с ограничением wet/feedback, RMS matching и защитой sample peak.
- Версионированные манифесты ventusample.pack.v1 и v2 для явного обмена пакетами.
- Необязательная рабочая станция Qt 6 Widgets; desktop-сборка зависит от наличия Qt SDK.

## Архитектура и поток данных

~~~mermaid
flowchart LR
  P["Seed / profile / parameters"] --> G["Deterministic generator"]
  G --> D["Offline C11 DSP layer"]
  D --> X["C++20 audio core"]
  X --> W["PCM16 WAV assets"]
  X --> M["Versioned pack manifest"]
  Q["Optional Qt 6 UI"] --> P
~~~

Генератор передаёт параметры с фиксированным seed через offline DSP-ядро, формируя WAV-файлы с заданными ограничениями и версионированный манифест набора. Необязательный интерфейс Qt настраивает входные параметры, но не превращает движок в аудиохост реального времени.

## Требования

CMake 3.21+, компилятор C++20 и компилятор C для DSP-слоя C11. Qt 6 Widgets необязателен для desktop-приложения; переносимое ядро и его тесты собираются без Qt. В Linux рекомендуется Ninja.

## Сборка

~~~sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

### Переносимое ядро без Qt

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

## Запуск

Desktop-цель создаётся только при обнаружении Qt 6 Widgets. В противном случае используйте сборку portable core.

## Интерфейсы

| Surface | Contract | Boundary |
|---|---|---|
| Presets | `ventusample.preset.v1` | Schema-validated settings |
| Pack exchange | `ventusample.pack.v1 / v2` | JSON manifest plus WAV assets |
| Audio output | `RIFF PCM16 stereo WAV` | Bounded local file output with peak/RMS metadata |
| Plugin layer | Native offline DSP graph | No real-time VST/AU hosting |

## Безопасность и границы доверия

Рендеринг по умолчанию выполняется offline; фоновый захват аудио и сетевая телеметрия не входят в аудиоконвейер. Импортируемые WAV считаются недоверенными и проверяются по формату, чанкам, кадрам и размеру. Внешние нативные библиотеки не загружаются неявно. Управление драйверами, прошивкой и системным звуком не выполняется.

Импортированные файлы следует считать недоверенными данными. Перед обработкой проверяйте размер файлов, границы чанков и заявленные форматы.

## Проверка качества

~~~sh
ctest --test-dir build --output-on-failure
~~~

Тесты проверяют детерминированную генерацию, маршрутизацию плагинов, границы WAV, ресэмплинг, MIDI-файлы, offline-анализ спектра и транзакции среды выполнения. Тесты графического интерфейса добавляются только при наличии Qt Widgets. Проценты покрытия и fuzzing не заявляются без опубликованных отчётов.

## Доказательства производительности

Тесты генератора и DSP проверяют детерминированность и ограниченность обработки, а не задержку real-time-воспроизведения. Здесь не заявляются фиксированная задержка в микросекундах, объём RAM или матрица поддерживаемых sample rate.

## Совместимость

CMake предоставляет переносимое ядро и необязательную desktop-цель Qt 6 Widgets. Документированы пути сборки MSVC и GCC/Clang; доступность desktop зависит от совместимого SDK Qt 6.

## Документация и экосистема

- [Features](docs/FEATURES.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)
- [MIDI and resampling](docs/MIDI_AND_RESAMPLING.md)
- [Plugins](docs/PLUGINS.md)
- [Source integration and licensing](docs/SOURCE_INTEGRATION.md)
- [Documentation index](docs/README.md)

Документы аудиодвижка, MIDI/resampling, плагинов и лицензирования исходников определяют границы форматов и точки расширения.

Обмен наборами с Reaper использует файлы и версионированный формат. Необязательная интеграция с нативными выражениями Reaper включается только при явной передаче локального дерева исходников Reaper в CMake; нативные метрики Pytyliki остаются планируемой функцией с явным включением.

## Участие в разработке

Изучите [CONTRIBUTING.md](CONTRIBUTING.md) перед изменением кода или форматов. Сохраняйте изменения проверяемыми, добавляйте детерминированные тесты и используйте Conventional Commits, если это требуется.

## Лицензия и правовые условия

Распространяется по Apache-2.0. Программное обеспечение предоставляется «КАК ЕСТЬ» в пределах условий лицензии. Сторонние названия и товарные знаки принадлежат их владельцам.

Условия облачного сервиса, cookies, CORS и OAuth не применяются к документированной локальной среде выполнения.

---

<p align="center"><sub>Psychoporter 666 README Standard v1.0 · Ivan-VHS · 2026-11-25 UTC</sub></p>
