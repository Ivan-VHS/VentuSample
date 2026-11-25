# VentuSample — процедурная рабочая станция сэмплов

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

[![CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)

<p align="center"><img src="assets/readme/dashboard.svg" alt="Обзор VentuSample" width="100%"></p>
<p align="center"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="Desktop-приложение VentuSample" width="100%"></a></p>

VentuSample — нативная рабочая станция процедурных сэмплов на базе desktop-приложения C++20/Qt 6 и автономного аудиоядра для offline-обработки.

**Границы:** локальная генерация звука, настройка параметров, offline DSP и экспорт файлов. Это не real-time DAW, VST/AU-хост, облачный сервис или аудиодрайвер.

## Возможности

- Процедурная генерация сэмплов, пресеты и слоты A/B для сравнения параметров в сессии.
- Offline DSP, импорт/экспорт WAV, инструменты MIDI и преобразования частоты дискретизации.
- Plugin SDK и версионированные pack-манифесты; обнаружение и обмен учитывают метаданные и границы доверия.
- Воспроизводимые настройки генерации и ограниченная обработка файлов.

Документы: [Возможности](docs/FEATURES.md) · [Архитектура](docs/ARCHITECTURE.md) · [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md) · [MIDI и resampling](docs/MIDI_AND_RESAMPLING.md) · [Плагины](docs/PLUGINS.md) · [Права на исходники](docs/SOURCE_INTEGRATION.md).

## Сборка и тесты

Требуются CMake, компилятор C++20 и Qt 6 Widgets для desktop-приложения. Переносимое ядро собирается без Qt.

Linux:

```sh
sudo apt-get update
sudo apt-get install -y cmake ninja-build g++ qt6-base-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Переносимое ядро без Qt:

```sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

Windows использует MSVC и Qt 6:

```sh
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

## Спецификации и интеграции

- Формат пресетов: [схема ventusample.preset.v1](schemas/ventusample.preset.v1.schema.json).
- [Индекс документации](docs/README.md) · [Проверка pack для Reaper](docs/REAPER_PACK_AUDIT.md) · [Релизы](docs/RELEASE.md).
- [Общие стандарты экосистемы](https://github.com/Ivan-VHS/DAK/blob/main/docs/SHARED_STANDARDS.md).

[Вклад в проект](CONTRIBUTING.md) · [Безопасность](SECURITY.md) · [Apache-2.0](LICENSE)
