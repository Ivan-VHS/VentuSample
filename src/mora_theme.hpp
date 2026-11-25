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
    // Retain the public parameter for source compatibility; Mora now uses one
    // coherent Imperial Noir & Glass profile across the workstation.
    (void)dark;
    return {
        "#050505", "#101010", "#FFFFFF", "#D8D8D8", "#A3A3A3",
        "#343434", "#FFD700", "#FFE36E", "#070707",
        "#FF7777", "#070707", "#3B3000", "#FFD700"
    };
}

} // namespace mora::ui
