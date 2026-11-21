# QMR-20 — quality, cohesion and rhythm

Implemented acceptance framework for the offline C++20 renderer. “Monolith” means one coherent render contract, not one giant source file. Pack v1 compatibility is retained.

| # | Quality | Cohesion | Rhythm | Acceptance rule |
|---:|---|---|---|---|
| 01 | Качество | Монолитность | Ритмичность | validate → render → master → export → test |
| 02 | Качественность | Цельность | Размеренность | stable bar duration and bounded settings |
| 03 | Добротность | Целостность | Пульсация | consistent transients on stable pulse |
| 04 | Добротность исполнения | Единство | Ритмизация | shared drum/melody swing map |
| 05 | Безупречность | Нераздельность | Синкопированность | offbeat stays inside meter |
| 06 | Совершенство | Неделимость | Метрика | BPM/bar/16-step timing grid |
| 07 | Мастерство | Сплочённость | Тактовость | phrase accents reinforce bars |
| 08 | Профессионализм | Слитность | Метричность | one subdivision convention |
| 09 | Точность | Консолидация | Периодичность | seeded repetition and fixed phrase turns |
| 10 | Чистота | Стабильность | Цикличность | linked stereo gain |
| 11 | Надёжность | Непрерывность | Равномерность | expected frame count |
| 12 | Выверенность | Однородность | Мелодичность | melody follows shared swing grid |
| 13 | Исключительность | Согласованность | Каденция | restrained 4-bar turnaround |
| 14 | Превосходство | Системность | Динамичность | density controls fills |
| 15 | Искусность | Компактность | Согласованность | fill scheduler avoids duplicated events |
| 16 | Отточенность | Нерасчленённость | Повторяемость | one parameter snapshot + seed |
| 17 | Филигранность | Сопряжённость | Текучесть | swing and humanization remain distinct |
| 18 | Высококлассность | Сбалансированность | Раскачка | no independent L/R compressor pumping |
| 19 | Основательность | Фундаментальность | Поступательность | offline, bounded, portable pipeline |
| 20 | Эталонность | Непоколебимость | Ритмика | tests protect duration, seed, swing, peak |

## Implemented gates

- One seed and immutable parameter snapshot govern the render.
- Drum and melodic accents share swing timing; humanization is separate.
- Phrase turns may occur every 4 bars.
- Stereo compressor uses one detector/gain for both channels.
- Sample-peak ceiling ≈−0.3 dBFS; not oversampled true peak.
- Manifest stores settings and per-file peak/RMS/crest; WAV output remains PCM16.
- QMR is not a substitute for listening or user judgment.
