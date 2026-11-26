# VentuSample — офлайн робоча станція процедурного аудіо

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

> Детермінована робоча станція C++20 для генерації семплів і offline DSP з обмеженим імпортом/експортом PCM16 WAV, версіонованими pack-manifest і необов’язковою оболонкою Qt 6.

**Специфікація**

| Поле | Поточний запис |
|---|---|
| Репозиторій | [Ivan-VHS/VentuSample](https://github.com/Ivan-VHS/VentuSample) |
| Версія проєкту | 0.2.0 (контракт проєкту та CMake) |
| Стан проєкту | Розробка; offline-аудіоядро |
| Ядро | C++20; C11 DSP compatibility layer |
| Desktop | Необов’язкова оболонка Qt 6 Widgets |
| Збірка | CMake 3.21+ |
| Аудіовихід | PCM16 WAV, детермінований локальний рендеринг |
| Pack-формати | Manifest v1 та v2 |
| Ліцензія | [Apache-2.0](LICENSE) |
| Перевірка документації | 2026-11-26 |

<p align="center"><img src="assets/readme/ventusample-workstation.svg" alt="Робоча станція VentuSample" width="100%"></p>

## Зміст
- [Призначення та архітектура](#призначення-та-архітектура)
- [Збірка та перевірка](#збірка-та-перевірка)
- [Аудіо- та файлові контракти](#аудіо-та-файлові-контракти)
- [DSP і система плагінів](#dsp-і-система-плагінів)
- [Інтеграції](#інтеграції)
- [Обмеження та права](#обмеження-та-права)
- [Документація та версії](#документація-та-версії)

## Призначення та архітектура

VentuSample генерує відтворювані sample packs і локально обробляє аудіофайли. Ядро C++20 відповідає за перевірку параметрів, детерміновану генерацію патернів, stereo float processing, offline DSP-маршрутизацію, WAV I/O та створення manifest. Desktop-ціль Qt 6 Widgets необов’язкова й збирається лише за наявності SDK.

~~~mermaid
flowchart TB
  U["Параметри / seed / variation"] --> V["Перевірка та ресурсні обмеження"]
  V --> G["Планувальник патернів і рендерер"]
  G --> F["Stereo float buffer"]
  F --> D["Offline DSP / stochastic router"]
  D --> M["Master і peak guard"]
  M --> W["PCM16 WAV assets"]
  V --> J["Версіонований pack manifest v1/v2"]
  W --> P["Каталог пакета"]
  J --> P
  Q["Необов’язковий Qt 6 desktop"] --> U
~~~

Рендеринг виконується offline у робочому потоці. Real-time audio callback і доступ до аудіопристрою не надаються.

## Збірка та перевірка

Потрібні CMake 3.21+ і компілятор C++20. Для desktop-виконуваного файла встановіть Qt 6 Widgets.

### Ядро та доступні тести

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

Без Qt 6 окремо зберіть ядро та його тести:

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

UI-тести Qt доступні лише коли налаштовано Qt Widgets target. Пропуск desktop-target не означає успішну збірку desktop-застосунку.

## Аудіо- та файлові контракти

| Поверхня | Контракт |
|---|---|
| Створені assets | RIFF/WAVE PCM16 |
| Імпортоване джерело | PCM16 mono/stereo WAV |
| Частота джерела | 8–192 кГц |
| Межі джерела | ≤128 MiB і ≤20 000 000 frames |
| Chop | Імпорт чотирьох зрізів і розміщення за beat |
| Pitch shift | ±12 півтонів через resampling; тривалість/швидкість змінюється |
| Частоти експорту | 22,05; 44,1; 48 або 96 кГц |
| Pack manifests | v1 і v2; v2 додає необов’язкові tonal/routing metadata |
| Reaper audit | Читає metadata manifest; WAV-файли не змінюються |

Усе імпортоване аудіо є недовіреним вводом і проходить перевірку формату, чанків, кількості кадрів і розміру. Тональні мітки описують налаштування генератора, а не автоматичне визначення тональності імпортованого матеріалу. Drum assets не отримують вигаданих key labels.

## DSP і система плагінів

Поточний проєкт містить seed-based процедурні профілі, спільний swing/timing schedule, Golden Era arrangement, обмежений stereo mastering, восьмиефектний stochastic offline router і каталог Quantum Audio Engine зі 133 іменованими module identities на основі 21 повторно використовуваного DSP kernel.

Stochastic router обробляє вже відрендерений аудіобуфер перед кодуванням WAV. Onset-trigger використовує event frames рендерера; він не виявляє транзієнти у довільному імпортованому аудіо. Quantum Audio Engine — offline DSP-каталог і ланцюжок обробки, а не фізичний квантовий комп’ютер чи real-time DAW engine.

Див. [Features](docs/FEATURES.md), [Architecture](docs/ARCHITECTURE.md), [Plugins](docs/PLUGINS.md), [Quality Monolith Rhythm](docs/QUALITY_MONOLITH_RHYTHM.md) та [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md).

## Інтеграції

VentuSample експортує manifest v1/v2 і WAV assets. Поточний Reaper consumer перевіряє та порівнює metadata pack; він не аналізує, не переписує і не рендерить waveform WAV. Прямий native metrics adapter Pytyliki запланований і має бути opt-in та пройти ABI differential tests перед заявою про доступність.

Міст Reaper у CMake вимкнений, доки явно не вказано локальний каталог вихідного коду Reaper. Зовнішня native-бібліотека автоматично не завантажується.

## Обмеження та права

> VentuSample — offline-робоча станція, а не real-time audio engine і не DAW plugin host.

Real-time playback, захоплення аудіопристрою, DAW plugin hosting, tempo-preserving time-stretch і GPU dispatch не реалізовані. Транспонування джерела використовує resampling, тому зміна висоти тону також змінює тривалість відтворення. Рендерер використовує portable scalar path; універсальні показники latency або throughput не заявляються.

Користувачі відповідають за права на імпортовані семпли, створені outputs і розповсюджувані packs. Репозиторій не заявляє права власності чи ліцензійної чистоти стороннього аудіоматеріалу. Не додавайте приватні sample paths або неліцензовані записи до публічних fixture.

## Документація та версії

Почніть з [індексу документації](docs/README.md) та [інформаційної архітектури](docs/INFORMATION_ARCHITECTURE.md).

- [Матриця функцій](docs/FEATURES.md)
- [Архітектура й аудіоконтракт](docs/ARCHITECTURE.md)
- [Посібник користувача](docs/USER_GUIDE_RU.md)
- [Manifest і Reaper pack audit](docs/REAPER_PACK_AUDIT.md)
- [Plugin API та lifecycle](docs/PLUGINS.md)
- [MIDI та resampling](docs/MIDI_AND_RESAMPLING.md)
- [Quality Monolith Rhythm](docs/QUALITY_MONOLITH_RHYTHM.md)
- [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)
- [Контроль документації](docs/DOCUMENTATION_CONTROL.md)

Контракт проєкту та CMake декларують версію 0.2.0; це саме собою не підтверджує стабільний реліз. Зміни публічного pack-формату потребують оновлення схем, тестів сумісності й приміток щодо міграції. Перед зміною DSP-input, плагінів або pack-контрактів прочитайте [CONTRIBUTING.md](CONTRIBUTING.md) і [SECURITY.md](SECURITY.md). Ліцензія — [Apache-2.0](LICENSE); зберігайте повідомлення сторонніх компонентів.