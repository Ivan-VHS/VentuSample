# VentuSample — процедурна робоча станція семплів

## Версійні пресети та порівняння A/B

Десктопний застосунок зберігає та завантажує локальні JSON-пресети формату `ventusample.preset.v1`. Вони містять музичний профіль, ритм, фільтри, семплер і маршрутизацію DSP. Випадковий seed за замовчуванням не додається: пресет описує налаштування звучання, а не фіксує кожну згенеровану варіацію; під час збереження seed можна додати окремо.

Використовуйте **Presets → Save preset** або **Presets → Load preset**. Два додаткові слоти A і B дають змогу зберігати й перемикати набори параметрів у межах поточного сеансу без запису на диск. Перед прийняттям пресет перевіряється за тими самими правилами, що й генератор. Локальні шляхи до вихідних семплів навмисно не експортуються, оскільки залежать від конкретної машини.

Схему опубліковано в `schemas/ventusample.preset.v1.schema.json`. Пресети залишаються локальними файлами: хмарної синхронізації немає, а право власності на описане ними аудіо не заявляється.

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

<p align="center"><img src="assets/readme/dashboard.svg" alt="Огляд проєкту VentuSample" width="100%"></p>

[Вихідний код](https://github.com/Ivan-VHS/VentuSample) · [CI workflow](.github/workflows/ci.yml) · [Індекс документації](docs/README.md) · [Apache-2.0](LICENSE)

## Реальний знімок desktop-застосунку

<p align="center"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="Реальний знімок VentuSample у Windows" width="100%"></a></p>
<p align="center"><sub>1920×1080 · Натисніть зображення, щоб відкрити оригінальний PNG.</sub></p>

Справжній скріншот VentuSample, запущеного на Windows CI desktop. Зображення захоплене зібраним Qt-застосунком; банер і схема потоку даних — окремі авторські SVG-ілюстрації.

### Архітектура одним поглядом

<p align="center"><img src="assets/readme/workflow.svg" alt="VentuSample implemented data flow" width="100%"></p>

Реальний шлях: параметри й seed → C++20 renderer → stereo buffer → необов’язковий офлайн-DSP → PCM16 WAV і JSON manifest.


[![C++20 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=C%2B%2B20%20CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml)  [![Qt desktop jobs](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=Qt%20desktop%20jobs)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml)  [![License](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)


> **Призначення:** VentuSample — нативна робоча станція на C++20 для детермінованої процедурної генерації семпл-паків і офлайн-обробки WAV. За наявності Qt доступний desktop-інтерфейс Qt 6; ядро DSP можна збирати окремо. Це не hosted web demo, browser sandbox, DAW, host аудіопристроїв або production audio service. Документацію звірено з `main` **22.11.2026**.

## Призначення та межі

VentuSample створює локальні hip-hop sample packs на основі явних параметрів і seed. Застосунок рендерить аудіо, обробляє підтримувані WAV через офлайн-DSP та записує версійовані JSON-manifest із налаштуваннями й даними про результат. DAW, аудіоінтерфейс, мережа чи hosted backend не потрібні.

Цільова аудиторія — beatmakers, sound designers, розробники аудіо, QA-інженери та рев’юери, яким потрібна детермінована й перевірювана генерація семплів та повторювана офлайн-обробка.

### Реалізовані можливості

| Напрям | Реалізована поведінка | Межа |
|---|---|---|
| Процедурний генератор | Full Loop, Melody, Drums, Bass та One-Shot Kit | локальний рендеринг; однакові seed і налаштування відтворюють той самий результат |
| Експорт аудіо | stereo PCM16 WAV і версійований JSON-manifest | локальні файли; наявні файли не перезаписуються |
| Імпорт WAV | підтримувані вихідні PCM16 mono/stereo файли | перевіряються розмір, кількість кадрів, sample rate і формат |
| Quantum Audio Engine | 133 типізовані ідентифікатори модулів та 21 повторно використовуване DSP-ядро | творча офлайн-DSP, не фізичні квантові обчислення |
| Migrated C11 processing | обмежений WAV I/O, фільтри, компресор, насичення, limiter і вимірювання | окрема C11-ціль, офлайн-буфери |
| Offline spectrum analysis | FFT-пік, спектральний центроїд і 85% rolloff наявного буфера | обмежений FFT 256–65 536 точок; без доступу до пристроїв |
| Ресемплінг | обмежена лінійна інтерполяція для mono/stereo буферів | якість для попереднього перегляду; без anti-alias фільтрації |
| MIDI inspection | розбір Standard MIDI File формату 0/1, нот, темпу та розміру такту | обмежений read-only parser; без MIDI-пристроїв і відтворення |
| Stochastic Router | необов’язкова seeded-маршрутизація через вісім вбудованих процесорів | офлайн-обробка буфера; вимкнено за замовчуванням |
| Plugin Manager | 19 вбудованих інструментів і перевірений C++20 handler API | виявлення manifest — лише метадані; без довільного DLL/SO loading |
| Atomic-Quantum 133 | окрема обмежена класична симуляція | без доступу до квантового обладнання |
| Reaper bridge | необов’язковий C17 integer-expression ABI під час збірки | явний локальний шлях до source; не є частиною audio DSP |

14 профілів генератора: Boom Bap, Billionaire Golden Era, East Coast, Harlem Dip-Trap, G-Unit, Modern Trap, Atlanta Trap, UK Drill, UK Grime, Memphis, Jazz Rap, G-Funk, Boom Trap і Cripple Trill.

Застосунок не є real-time аудіорушієм. Рендеринг і DSP виконуються офлайн у worker threads; немає callback аудіопристрою, playback hot-swap, VST/AU host, довільного завантажувача сторонніх плагінів чи DAW automation.

Скріншот Windows нижче — знімок desktop-застосунку, а не браузерне прев’ю.


## Швидкий старт

### Вимоги

- CMake **3.21 або новіший**.
- Компілятор C++20: GCC, Clang або MSVC.
- Git.
- Qt **6 Widgets** для desktop-застосунку. Бібліотеку DSP/core і non-Qt tests можна збирати без Qt.
- Ninja необов’язковий, але використовується Linux desktop job у CI.

Для рендерингу аудіо проєкту не потрібні Python чи Node.js. Python 3.10–3.12 використовується в CI для перевірки project contract та engineering validators.

### Linux: збирання desktop-робочої станції

В Ubuntu 24.04 встановіть ті самі пакети toolchain, що й CI:

```sh
sudo apt-get update
sudo apt-get install -y cmake ninja-build g++ qt6-base-dev
```

Налаштуйте, зберіть і запустіть тести:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Desktop executable створюється налаштованою збіркою CMake. Для Linux-збірки Ninja запустіть `./build/VentuSample`; для Windows Release — `build\\Release\\VentuSample.exe`.

### Переносиме ядро без Qt

Якщо Qt 6 Widgets не встановлено або потрібні лише DSP engine та тести:

```sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

Якщо `VENTU_BUILD_DESKTOP=ON`, але Qt 6 Widgets не знайдено, CMake виведе warning і пропустить desktop target; це не означає, що GUI executable було створено.

### Windows: збирання з MSVC і Qt 6

Використовуйте Visual Studio 2022 x64 developer environment та встановіть Qt 6 Widgets (поточний CI використовує Qt **6.8.3**, MSVC 2022 x64):

```sh
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Windows CI встановлює Qt і налаштовує відповідний MSVC toolchain перед виконанням цих команд.

### Перша перевірка

Успішний `ctest` є загальною перевіркою збірки. Тести покривають валідацію параметрів, детерміновану повторюваність, варіації, WAV round-trip, stochastic DSP, схеми, життєвий цикл плагінів і поведінку Quantum Audio Engine.

Стандартного HTTP-порту, localhost URL, запуску браузера, seed бази даних чи віддаленого mock service немає. Генерація й обробка виконуються локально.

## Відтворювані дані та тестові сценарії

VentuSample не використовує віддалену базу даних або live API. Генератор працює з локальними параметрами та seed; імпортоване аудіо надходить із файлу, вибраного оператором. Manifest створюється локально поряд із вихідними файлами.

Перевірте такі сценарії у desktop-застосунку:

1. **Повторюваний loop:** виберіть profile генератора, BPM, key/scale та фіксований seed; двічі створіть Full Loop з однаковими налаштуваннями й порівняйте аудіо та manifest.
2. **Окремі stems:** створіть Melody, Drums і Bass окремо та перевірте тип кожного результату.
3. **One-shot kit:** згенеруйте набір із восьми звуків і перевірте, що tonal labels додаються лише звукам із висотою тону.
4. **Імпорт WAV:** завантажте підтримуваний PCM16 mono/stereo файл і перегляньте його властивості.
5. **Chop і pitch:** виберіть до чотирьох фрагментів і застосуйте описаний semitone resampling; це одночасно змінює висоту й швидкість, а не виконує time-stretch.
6. **Офлайн DSP-ланцюг:** завантажте підтримуваний WAV у Quantum Audio Engine, сформуйте впорядкований ланцюг, змініть параметри модулів і експортуйте оброблений WAV.
7. **Stochastic routing:** увімкніть необов’язковий router, виберіть topology/trigger/intensity і виконайте новий рендер. Seed, variation, trigger та effect mask визначають маршрут.
8. **Виявлення плагіна:** перегляньте локальний `plugin.json`; виявлений запис метаданих залишається вимкненим, доки явно не підключено перевірений native handler.

### Діапазони генератора та обмеження файлів

| Параметр / вхід | Підтримуваний діапазон |
|---|---|
| BPM / bars / variations | 60–190 / 1–16 / 1–32 |
| Key / scale | C–B / 6 scale families |
| Swing | 50–68% |
| Density / grit / saturation / stereo | 0–100% |
| High-pass / low-pass | 20–300 Hz / 2–20 kHz |
| Snare offset | −30…+30 ms |
| Bus compression | 1:1–12:1 |
| Sample rates | 22.05 / 44.1 / 48 / 96 kHz |
| Imported source | PCM16 mono/stereo, 8–192 kHz |
| Source-file limits | 128 MiB і 20 000 000 frames |
| Chop pitch / mix | −12…+12 semitones / 0–100% |

Low-pass cutoff має бути вищим за high-pass cutoff і нижчим за 0.49× sample rate. Імпортоване аудіо автоматично не аналізується на тональність.

### Контракт Stochastic Router

Router необов’язковий і **вимкнений за замовчуванням**. Він обробляє rendered stereo buffers перед кодуванням PCM16 WAV. Вбудований пул містить low-pass, high-pass, saturator, bitcrusher, delay, reverb, ring modulation і stereo width.

| Параметр | Контракт |
|---|---|
| Topology | hybrid, serial або parallel; parallel використовує 2–4 обмежені гілки |
| Trigger | на файл, на такт або renderer-scheduled event frame |
| Intensity | 0–100%; нульова інтенсивність прозора |
| Wet / feedback | максимум 65% / 72% |
| Segment crossfade | 5–20 ms |
| RMS compensation | обмеження 0.5–2× |
| Output guard | sample peak ceiling не вище −0.3 dBFS |
| Repeatability | generator seed + variation + trigger index |

Event mode використовує scheduled frames, які генерує procedural renderer. Він не виявляє transient в довільних імпортованих WAV-файлах. Це офлайн signal processing, а не real-time callback чи plugin hot-swap.

### Тривалість стану та зберігання

Робочі аудіобуфери й поточний UI state існують у процесі застосунку. Вихідні WAV і JSON-manifest записуються до локального каталогу, вибраного користувачем; наявні файли не перезаписуються. Закриття програми завершує memory session, але не видаляє експортовані файли.

Desktop UI зберігає власні налаштування через native preferences layer. Немає browser localStorage, sessionStorage, IndexedDB, cookie state, спільної серверної бази або автоматичної cloud sync. Вбудованого latency/HTTP-500 simulator чи fallback для зовнішнього сервісу немає, оскільки audio path працює офлайн.

## Конфігурація, середовище та довіра

Кореневого `.env.example`, Docker Compose stack, системи акаунтів, demo credentials, API token, CORS configuration, reverse proxy або локального HTTPS немає. Генерація й обробка аудіо не потребують мережі.

Параметри збірки з `CMakeLists.txt`:

| CMake option | Типове значення | Призначення |
|---|---:|---|
| `VENTU_BUILD_DESKTOP` | `ON` | знайти Qt 6 Widgets і зібрати desktop target, якщо Qt доступний |
| `VENTU_BUILD_TESTS` | `ON` | зібрати native tests |
| `VENTU_ENABLE_SANITIZERS` | `OFF` | увімкнути AddressSanitizer/UndefinedBehaviorSanitizer для підтримуваних GCC/Clang builds |
| `REAPER_SOURCE_DIR` | порожній | явний локальний checkout Reaper; вмикає необов’язковий C17 expression bridge |

Reaper bridge вимкнений, якщо явно не задано локальний source directory. Він використовує native expression ABI Reaper для детермінованих integer control expressions; не замінює C++20 engine і не потрапляє в real-time DSP path.

### Межі безпеки плагінів

Plugin Manager має 19 вбудованих інструментів. Локальний `plugin.json` можна рекурсивно виявити як метадані, але знайдений запис залишається вимкненим, доки не підключено явно перевірений native handler. Менеджер не завантажує manifest entrypoints, довільні DLL/SO чи executable payload. Публічна схема manifest: [ivan-vhs.plugin.v1](schemas/ivan-vhs.plugin.v1.schema.json).

Не слід трактувати виявлення manifest як виконання коду або гарантію sandbox. Реєстрація handler — це явна native C++ операція, яку перевіряє застосунок.

## Архітектура та структура репозиторію

Потік даних: **параметри + seed → procedural renderer → stereo float buffer → optional offline routing/DSP → PCM16 WAV + версійований JSON-manifest**.

| Шлях | Вміст |
|---|---|
| `src/` | C++20 generator, renderer, audio buffers, DSP kernels, plugin manager, Qt desktop UI й optional Reaper bridge |
| `src/compat/pytyliki-dsp/` | перенесена бібліотека обробки буферів на C11 |
| `integrations/cockos-reaper/` | необов’язкові Lua-дії для Cockos REAPER |
| `tests/` | native DSP, generator, manifest, UI, plugin lifecycle та regression tests |
| `docs/` | архітектура, функції, plugin SDK, UI behavior та audio contracts |
| `docs/PYTYLIKI_DSP_MIGRATION.md` | власність і правила збирання перенесених C11-модулів |
| `docs/integrations/COCKOS_REAPER.md` | необов’язкові скрипти Cockos REAPER |
| `schemas/` | версійовані sample-pack і plugin manifest schemas |
| `tools/` | project-contract, fixture, visual-canon та engineering validators |
| `federation/` | явні interoperability й adapter manifests |
| `assets/screenshots/` | знімок desktop-застосунку |
| `.github/workflows/` | Python contract, native, Linux Qt і Windows Qt CI jobs |
| `CMakeLists.txt` | C++20 build graph і optional feature switches |
| `project-contract.json` | машинозчитуваний project contract |

Desktop використовує Qt 6 Widgets. Генератор і DSP engine написані на C++20. Python у CI перевіряє repository contracts; це не Python web backend. Немає frontend/backend deployment, web server, mock server, browser bundle, port binding, CORS чи public live preview. Browser support і responsive viewport rules не застосовуються.

## CI, артефакти та експлуатація

[CI workflow](.github/workflows/ci.yml) включає:

- Python contract checks на Python 3.10, 3.11 і 3.12;
- native C++20 core build і CTest без desktop target;
- Linux DSP + Qt desktop build на Ubuntu 24.04 з Qt 6 Widgets;
- Windows 11 native Qt desktop build на runner Windows 2022 із Qt 6.8.3 та MSVC 2022;
- project-contract, generated-fixture, engineering-evolution і CLife validation.

Windows desktop job захоплює запущений застосунок і завантажує screenshot artifact зі строком зберігання 30 днів. Цей workflow **не** оголошує public web deployment, hosted preview або tagged Windows release archive. Не слід виводити наявність release packaging чи автоматичного deployment із успішного CI.

Для відкату локальної зміни поверніться до попереднього Git commit або перебудуйте останню відому робочу ревізію. Немає онлайн-сервісу чи deployed database, які потрібно відкотити. Логи CI та артефакти доступні в GitHub Actions.

## Усунення проблем

| Симптом | Що перевірити |
|---|---|
| Відсутній Qt desktop target | встановіть Qt 6 Widgets або налаштуйте `-DVENTU_BUILD_DESKTOP=OFF` для core-only build |
| CMake відхиляє версію | встановіть CMake 3.21 або новіший |
| Помилка компіляції C++ | перевірте C++20 compiler і першу діагностику компілятора |
| Імпорт WAV відхиляється | перевірте PCM16, mono/stereo, 8–192 kHz, 128 MiB і 20M-frame limits |
| Експортований результат відрізняється | перевірте generator seed, variation і settings; для повторення stochastic route потрібні ті самі параметри |
| Router несподівано змінює imported audio | onset routing використовує renderer-scheduled events; він не виконує transient detection довільного imported WAV |
| Manifest плагіна видно, але він не запускається | discovery читає лише metadata; підключіть явно перевірений native handler |
| Недоступний Reaper bridge | задайте `REAPER_SOURCE_DIR` до наявного довіреного Reaper source checkout під час configure |

Для помилок тестів або збірки збережіть точну CMake-команду, версії compiler/Qt, назву failing test і повний GitHub Actions log.

## Відомі обмеження, внесок і ліцензія

VentuSample — офлайн-робоча станція семплів, а не DAW або real-time audio host. Генератор створює PCM16 stereo WAV і manifest; він не звертається до зовнішніх API, не стримить аудіо в браузер і не керує cloud projects. Stochastic router використовує вісім вбудованих процесорів і не хостить сторонні плагіни. Quantum Audio Engine — творчий DSP-каталог; Atomic-Quantum — окрема класична симуляція, не фізичні квантові обчислення. Для імпорту й експорту аудіо потрібні відповідні права на вихідний матеріал; хеш підтверджує ідентичність байтів, але не право власності.

Правила розробки, перевірки змін і оформлення Pull Request описані в [CONTRIBUTING.md](CONTRIBUTING.md). Пропонуйте зміни через [GitHub Issues](https://github.com/Ivan-VHS/VentuSample/issues) або Pull Request. Додавайте ОС, версії compiler/CMake/Qt, seed та точні налаштування, безпечний опис вхідного WAV і точну команду/результат. Перед надсиланням запускайте CTest і project-contract checks.

Ліцензія: [Apache-2.0](LICENSE).

## Відповідність 66 правилам README

Чекліст вебстенду зіставлено з реальним офлайн desktop/audio продуктом; суто веброзділи явно позначені як непридатні:

| Правила | Покриття в README |
|---|---|
| 1–11 · призначення та межі | пояснено ціль детермінованої sample workstation, аудиторію, робочі сценарії, офлайн-область і межі функцій |
| 12–22 · швидкий старт | наведено вимоги CMake/C++20/Qt, Linux/Windows/core-only команди, build і CTest |
| 23–33 · конфігурація та доступ | перераховано реальні CMake options і optional Reaper bridge; немає фальшивих ENV, акаунтів, секретів, CORS чи HTTPS |
| 34–44 · дані та поведінка | описано повторюваність seed, локальні WAV input/output, manifests, file limits, plugin discovery та UI state lifetime |
| 45–55 · архітектура та збірка | наведено реальну C++20/Qt архітектуру, DSP-етапи, каталоги репозиторію, схеми та CI matrix |
| 56–66 · експлуатація та підтримка | додано реальний CI workflow, межі screenshot artifact і rollback, troubleshooting, відомі проблеми, шлях внеску й ліцензію |

**Публічного URL VentuSample web-stand немає.** Desktop-скріншот і CI screenshot artifact не є інтерактивним браузерним демо.

## Єдина оболонка звіту

JSON-результат можна обгорнути в загальний формат метаданих ivan-vhs.report.v1:

    python tools/report_envelope.py --tool ventusample --version VERSION < render-manifest.json

Див. docs/REPORT_ENVELOPE.md і schemas/ivan-vhs.report.v1.schema.json. Результат самого інструмента залишається без змін.


## Карта документації

Використовуйте [індекс документації](docs/README.md) як точку входу, а [карту інформаційної архітектури](docs/INFORMATION_ARCHITECTURE.md) — для навігації аудіо/DSP, робочими процесами Studio, пресетами й плагінами, архітектурою, UX та інтеграціями. Хаби ведуть до детальних контрактів; підтримувані формати й стан реалізації підтверджуються кодом, тестами, manifest-файлами та схемами.


## Інтеграція з робочим столом Mora

VentuSample доступний зі спільного робочого простору Mora після збирання desktop target на Qt 6. Mora надає каталог і початкову тему, а генерація семплів та офлайн-DSP залишаються функціями самого VentuSample. Див. [інтеграцію Mora](docs/MORA_DESKTOP_INTEGRATION.md) та [контракт DuckOS](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/MORA_DESKTOP.md).
