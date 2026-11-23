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

inline Palette palette() noexcept {
    return {
        "#080808", "#151515", "#FFFFFF", "#E5E5E5", "#A6A6A6",
        "#3A3A3A", "#FFD400", "#FFE45C", "#080808",
        "#FFD400", "#080808", "#3A3100", "#FFE45C"
    };
}

} // namespace mora::ui
