<p align="center">[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)</p>

<h1 align="center"><strong>🎛️ VentuSample — робоча станція процедурного аудіо</strong></h1>

[![CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Release](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/release.yml?branch=main&label=Release)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/release.yml) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)

> **Детермінована генерація семплів та offline DSP. Без імітації real-time-рушія.**

**Стан:** Розробка · офлайн-аудіоядро реалізовано · Qt desktop частково · **Версія:** 0.2.0 · **License:** Apache-2.0 · **Стандарт README:** Psychoporter 666 v1.0 · **Дата перевірки:** 2026-11-25 UTC · **Source:** [Ivan-VHS/VentuSample](https://github.com/Ivan-VHS/VentuSample)

<p align="center"><img src="assets/readme/dashboard.svg" alt="VentuSample project overview" width="100%"></p>
<p align="center"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample desktop application screenshot" width="100%"></a></p>

## Швидка навігація

[🚀 Demo: Studio screenshot](assets/screenshots/ventusample-studio.png) · [Documentation](docs/README.md) · [Source](https://github.com/Ivan-VHS/VentuSample) · [Issues](https://github.com/Ivan-VHS/VentuSample/issues)

## Призначення та межі

Нативна робоча станція процедурних семплів із ядром C++20, сумісним DSP-шаром C11 та необов’язковим desktop-застосунком Qt 6 Widgets. Рендеринг виконується offline. Це не real-time DAW, VST/AU-хост, хмарний сервіс або аудіодрайвер.

## Можливості

- Детермінована процедурна генерація стерео за 14 документованими hip-hop і суміжними профілями.
- Перезбирання golden-era sample crates, зважений добір барабанних тембрів і відтворювані варіації.
- Обмежений імпорт/експорт PCM16 WAV, розміщення чотирьох зрізів і транспонування півтонами.
- Offline DSP-маршрутизація з обмеженням wet/feedback, RMS matching і захистом sample peak.
- Версіоновані маніфести ventusample.pack.v1 та v2 для явного обміну пакунками.
- Необов’язкова робоча станція Qt 6 Widgets; desktop-збирання залежить від наявності Qt SDK.

## Архітектура та потік даних

~~~mermaid
flowchart LR
  P["Seed / profile / parameters"] --> G["Deterministic generator"]
  G --> D["Offline C11 DSP layer"]
  D --> X["C++20 audio core"]
  X --> W["PCM16 WAV assets"]
  X --> M["Versioned pack manifest"]
  Q["Optional Qt 6 UI"] --> P
~~~

The generator sends seeded settings through the offline DSP core into bounded WAV assets and a versioned pack manifest. The optional Qt interface configures inputs; it does not turn the engine into a real-time audio host.

## Вимоги

CMake 3.21+, компілятор C++20 і компілятор C для DSP-шару C11. Qt 6 Widgets необов’язковий для desktop-застосунку; переносне ядро та його тести збираються без Qt. У Linux рекомендовано Ninja.

## Збирання

~~~sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

### Portable core without Qt

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

## Запуск

Desktop-ціль створюється лише за наявності Qt 6 Widgets. В іншому разі використовуйте збирання portable core.

## Інтерфейси

| Surface | Contract | Boundary |
|---|---|---|
| Presets | `ventusample.preset.v1` | Schema-validated settings |
| Pack exchange | `ventusample.pack.v1 / v2` | JSON manifest plus WAV assets |
| Audio output | `RIFF PCM16 stereo WAV` | Bounded local file output with peak/RMS metadata |
| Plugin layer | Native offline DSP graph | No real-time VST/AU hosting |

## Безпека та межі довіри

Рендеринг за замовчуванням виконується offline; фонове захоплення аудіо та мережева телеметрія не входять до аудіоконвеєра. Імпортовані WAV вважаються недовіреними й перевіряються за форматом, чанками, кадрами та розміром. Зовнішні нативні бібліотеки не завантажуються неявно. Керування драйверами, прошивкою та системним звуком не виконується.

Imported assets must be treated as untrusted data. Validate file sizes, chunk boundaries and declared formats before consuming them.

## Перевірка якості

~~~sh
ctest --test-dir build --output-on-failure
~~~

The test targets exercise deterministic generation, plugin routing, WAV boundaries, resampling, MIDI files, offline spectrum analysis and runtime transactions. Desktop UI tests are registered only when Qt Widgets is available. Coverage and fuzzing percentages are not claimed without published reports.

## Докази продуктивності

Тести генератора й DSP перевіряють детермінованість та обмеженість обробки, а не затримку real-time-відтворення. Тут не заявляються фіксована затримка в мікросекундах, обсяг RAM або матриця підтримуваних sample rate.

## Сумісність

CMake надає переносне ядро й необов’язкову desktop-ціль Qt 6 Widgets. Задокументовано шляхи збирання MSVC і GCC/Clang; доступність desktop залежить від сумісного SDK Qt 6.

## Документація та екосистема

- [Features](docs/FEATURES.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)
- [MIDI and resampling](docs/MIDI_AND_RESAMPLING.md)
- [Plugins](docs/PLUGINS.md)
- [Source integration and licensing](docs/SOURCE_INTEGRATION.md)
- [Documentation index](docs/README.md)

Документи аудіорушія, MIDI/resampling, плагінів і ліцензування вихідних матеріалів визначають межі форматів і точки розширення.

Reaper pack exchange is file-based and versioned. Optional Reaper native expression integration is enabled only when a local Reaper source tree is explicitly supplied to CMake; Pytyliki native metrics remain a planned opt-in path.

## Участь у розробці

Ознайомтеся з CONTRIBUTING.md перед зміною коду або форматів. Робіть зміни перевірними, додавайте детерміновані тести та використовуйте Conventional Commits, якщо цього вимагає посібник.

## Ліцензія та правові умови

Поширюється за Apache-2.0. Програмне забезпечення надається «ЯК Є» в межах умов ліцензії. Сторонні назви й торговельні марки належать їхнім власникам.

Hosted-service terms, cookies, CORS and OAuth are not applicable to the documented local runtime.

---

<p align="center"><sub>Psychoporter 666 README Standard v1.0 · Ivan-VHS · 2026-11-25 UTC</sub></p>