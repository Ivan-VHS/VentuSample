# VentuSample — UI/UX implementation profile

**Shared standard:** [UI/UX Codex 40026](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/UI_UX_CODEX_40026.md).

This file keeps only the Qt workstation toolkit notes and implementation evidence. The shared specification owns common interaction rules, accessibility budgets, theme/component contracts and the definition of done.

### VentuSample — локальная реализация

- Основной путь остаётся: выбрать звуковую задачу → настроить параметры → выбрать источник/папку → создать pack → проверить WAV и manifest.
- Все поля, слайдеры, вкладки и кнопки получают доступные имена; часто используемые кнопки и поля сохраняют удобную область попадания и видимый focus state.
- В интерфейсе есть честное состояние генерации: indeterminate progress не выдаёт фиктивный процент, а завершение подтверждается только по результату worker.
- Список файлов и журнал ограничены/виртуализируются; двойной запуск генерации предотвращён; результаты доступны без потери контекста настроек.
- Qt timers/animations не запускаются в DSP callbacks, audio buffers или генераторном worker; только UI thread получает визуальную обратную связь.

## Implementation evidence — VentuSample

The Qt UI policy tests cover contrast calculations and ordinary/large-text thresholds. Accessible names are supplied for choices, parameter controls, progress, status, generated files and diagnostic output; field labels are linked to their corresponding editors. Manual keyboard-only, high-DPI and 200% scaling audits remain release checks. The UI policy does not report fabricated generator progress and does not run in the DSP/worker path.
