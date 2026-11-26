#pragma once

namespace mora::ui {

inline constexpr unsigned long nativeBackdropAttribute = 38;
inline constexpr unsigned long nativeBackdropMainWindow = 2;

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
            "#050505", "#101010", "#FFFFFF", "#E5E5E5", "#A6A6A6",
            "#3A3A3A", "#A78BFA", "#C1AEFF", "#1D1730",
            "#FF8EAC", "#330A1A", "#242039", "#6CD9C2"
        };
    }
    return {
        "#F5F3FA", "#FFFFFF", "#211F31", "#4E4B63", "#66627A",
        "#D6D0E7", "#5B43B5", "#49359D", "#FFFFFF",
        "#A91F49", "#FFFFFF", "#E6E0F7", "#006F68"
    };
}

} // namespace mora::ui
