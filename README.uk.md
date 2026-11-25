# VentuSample — процедурна робоча станція семплів

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

[![CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)

<p align="center"><img src="assets/readme/dashboard.svg" alt="Огляд VentuSample" width="100%"></p>
<p align="center"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="Desktop-застосунок VentuSample" width="100%"></a></p>

VentuSample — нативна робоча станція процедурних семплів на базі desktop-застосунку C++20/Qt 6 та автономного аудіоядра для offline-обробки.

**Межі:** локальна генерація звуку, налаштування параметрів, offline DSP та експорт файлів. Це не real-time DAW, VST/AU-хост, хмарний сервіс або аудіодрайвер.

## Можливості

- Процедурна генерація семплів, пресети та слоти A/B для порівняння параметрів у межах сесії.
- Offline DSP, імпорт/експорт WAV, інструменти MIDI та перетворення частоти дискретизації.
- Plugin SDK і версійні pack-маніфести; виявлення та обмін враховують метадані й межі довіри.
- Відтворювані налаштування генерації та обмежена обробка файлів.

Документи: [Можливості](docs/FEATURES.md) · [Архітектура](docs/ARCHITECTURE.md) · [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md) · [MIDI і resampling](docs/MIDI_AND_RESAMPLING.md) · [Плагіни](docs/PLUGINS.md) · [Права на джерела](docs/SOURCE_INTEGRATION.md).

## Збирання й тести

Потрібні CMake, компілятор C++20 і Qt 6 Widgets для desktop-застосунку. Переносне ядро можна зібрати без Qt.

Linux:

```sh
sudo apt-get update
sudo apt-get install -y cmake ninja-build g++ qt6-base-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Переносне ядро без Qt:

```sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

Windows використовує MSVC та Qt 6:

```sh
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

## Специфікації та інтеграції

- Формат пресетів: [схема ventusample.preset.v1](schemas/ventusample.preset.v1.schema.json).
- [Індекс документації](docs/README.md) · [Перевірка pack для Reaper](docs/REAPER_PACK_AUDIT.md) · [Релізи](docs/RELEASE.md).
- [Спільні стандарти екосистеми](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/SHARED_STANDARDS.md).
- [Сходи еволюції проєкту](docs/PROJECT_LADDER.md) · [Поточний рівень](.github/project-ladder.v1.json)

[Внесок у проєкт](CONTRIBUTING.md) · [Безпека](SECURITY.md) · [Apache-2.0](LICENSE)
