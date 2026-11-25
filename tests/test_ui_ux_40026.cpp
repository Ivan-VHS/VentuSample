#include "ui_ux.hpp"
#include "../src/mora_theme.hpp"

#include <QColor>
#include <cassert>
#include <cmath>

int main() {
    const QColor white("#FFFFFF"), canvas("#050505"), gold("#FFD700"), red("#FF7777");
    const QColor muted("#A3A3A3"), blue("#79B8FF"), green("#6EDAA3");
    assert(ventu::ui_ux::meetsTextContrast(white, canvas));
    assert(ventu::ui_ux::meetsTextContrast(white, gold));
    assert(!ventu::ui_ux::meetsTextContrast(QColor("#808080"), white));
    assert(ventu::ui_ux::meetsTextContrast(QColor("#808080"), white, true));
    assert(ventu::ui_ux::contrastRatio(QColor(), white) == 0.0);
    assert(std::isfinite(ventu::ui_ux::contrastRatio(white, canvas)));

    const auto moraDark = mora::ui::palette(true);
    const auto moraLight = mora::ui::palette(false);
    assert(moraDark.page == moraLight.page);
    assert(moraDark.accent == moraLight.accent);
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraDark.ink), QColor(moraDark.page)));
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraDark.onAccent), QColor(moraDark.accent)));
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraDark.onSignal), QColor(moraDark.signal)));
    assert(ventu::ui_ux::contrastRatio(QColor(moraDark.focus), QColor(moraDark.page)) >= 3.0);
    assert(ventu::ui_ux::meetsTextContrast(green, canvas));
    assert(ventu::ui_ux::meetsTextContrast(red, canvas));
    assert(ventu::ui_ux::meetsTextContrast(blue, canvas));
    assert(ventu::ui_ux::meetsTextContrast(muted, canvas));
    return 0;
}
