# VentuSample — краткое руководство

## Экспорт

1. Выбери профиль и тип: Full Loop, Melody, Drums, Bass или One-Shot Kit.
2. Задай BPM, такты, вариации, ноту и лад.
3. Настрой swing, density, grit, saturation, stereo, фильтры, snare offset и compression.
4. При необходимости импортируй PCM16 WAV и выбери 4 среза, pitch и mix.
5. Экспортируй в папку. Существующие файлы не перезаписываются.

## Тональность

| Тип | Метка |
|---|---|
| Full Loop / Melody / Bass | нота + лад |
| Drums | отсутствует |
| One-Shot Kit | только pitched bass/pluck |

Примеры: `A_Minor`, `C_Major`, `D_Harmonic_Minor`, `F_Dorian`. Manifest v2 содержит `musical_key`, `mode_family`, `scale`. Метка описывает настройки генератора; импортированный WAV автоматически не анализируется.

## Стартовые карманы

| Профиль | BPM | Swing |
|---|---:|---:|
| East Coast / Boom Bap | 88–96 | 55–60% |
| Harlem / Dip-Trap | 90–104 | 56–62% |
| G-Unit | 88–94 | 54–59% |
| Billionaire Golden Era | 88–96 | 57–61% |
| Modern / Atlanta Trap | 140–160 | 55–62% |
| UK Drill | 138–145 | 54–60% |
| UK Grime | 136–142 | 55–61% |

Golden Era rebuild: порядок фрагментов 0–2–1–3, акцент kick/snare, сдержанные hats. Ручные BPM, swing, snare offset и density остаются доступны.

## Импорт и мастеринг

- RIFF/WAVE PCM16, mono/stereo, 8–192 kHz; ≤128 МиБ и ≤20 млн кадров.
- MP3, FLAC, 24-bit и float WAV не поддерживаются.
- Ресемплинг ±12 semitones меняет и pitch, и скорость; это не time-stretch.
- Swing общий для ударных и мелодических акцентов; humanization отдельно.
- Turnaround может появляться в конце каждой 4-тактовой фразы.
- Stereo-linked compressor сохраняет баланс каналов. Ceiling около −0.3 dBFS sample peak, не true-peak limiter.

Используй только материал, на который у тебя есть права импорта, преобразования и распространения.
