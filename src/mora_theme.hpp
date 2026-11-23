#pragma once

namespace mora::ui {

struct Palette {
    const char* page;
    const char* surface;
    const char* ink;
    const char* secondary;
    const char* muted;
    const char* border;
    const char* accent;
    const char* accentHover;
    const char* onAccent;
    const char* signal;
    const char* onSignal;
    const char* selection;
    const char* focus;
};

inline Palette palette(bool dark) noexcept {
    if (dark) {
        return {
            "#10142A", "#191E39", "#F5F4FF", "#D5D6EB", "#A8ADC8",
            "#3B4262", "#A78BFA", "#C1AEFF", "#1D1730",
            "#FF8EAC", "#330A1A", "#353054", "#38D4B7"
        };
    }
    return {
        "#F5F3FA", "#FFFFFF", "#211F31", "#4E4B63", "#66627A",
        "#D6D0E7", "#5B43B5", "#49359D", "#FFFFFF",
        "#A91F49", "#FFFFFF", "#E6E0F7", "#006F68"
    };
}

} // namespace mora::ui
