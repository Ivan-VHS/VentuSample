#pragma once

namespace mora::ui {

enum class Theme {
    Amber,
    Light,
    Dark,
};

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

inline Palette palette(Theme theme) noexcept {
    if (theme == Theme::Amber) {
        return {
            "#211913", "#33261D", "#FFF3E2", "#E7D2B8", "#C4A98A",
            "#75543A", "#D58B3D", "#EAA653", "#211913",
            "#B64A40", "#FFF3E2", "#60442E", "#F3C16B"
        };
    }
    if (theme == Theme::Dark) {
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

inline Palette palette(bool dark) noexcept {
    return palette(dark ? Theme::Dark : Theme::Light);
}

} // namespace mora::ui
