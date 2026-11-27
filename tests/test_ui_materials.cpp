#include "mora_theme.hpp"

#include <cassert>

static_assert(mora::ui::nativeBackdropAttribute == 38);
static_assert(mora::ui::nativeBackdropMainWindow == 2);

int main() {
    const auto light = mora::ui::palette(false);
    const auto dark = mora::ui::palette(true);
    assert(light.page != nullptr && dark.page != nullptr);
    assert(light.accent != nullptr && dark.accent != nullptr);
    return 0;
}
