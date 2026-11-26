#include "ui_ux.hpp"
#include "../src/mora_theme.hpp"

#include <QColor>
#include <cassert>
#include <cmath>

int main() {
    const QColor white("#FFFFFF"), navy("#0A1E46"), blue("#0039A6"), grey("#808080");
    assert(ventu::ui_ux::meetsTextContrast(white, navy));
    assert(ventu::ui_ux::meetsTextContrast(white, blue));
    assert(!ventu::ui_ux::meetsTextContrast(grey, white));
    assert(ventu::ui_ux::meetsTextContrast(grey, white, true));
    assert(ventu::ui_ux::contrastRatio(QColor(), white) == 0.0);
    assert(std::isfinite(ventu::ui_ux::contrastRatio(white, navy)));

    const auto moraDark = mora::ui::palette(true);
    const auto moraLight = mora::ui::palette(false);
    assert(QColor(moraDark.page) == QColor("#050505"));
    assert(QColor(moraDark.surface) == QColor("#101010"));
    assert(QColor(moraDark.ink) == QColor("#FFFFFF"));
    assert(QColor(moraDark.accent) == QColor("#A78BFA"));
    assert(QColor(moraDark.signal) == QColor("#FF8EAC"));
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraDark.onAccent), QColor(moraDark.accent)));
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraDark.onSignal), QColor(moraDark.signal)));
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraLight.onAccent), QColor(moraLight.accent)));
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraLight.onSignal), QColor(moraLight.signal)));
    assert(ventu::ui_ux::contrastRatio(QColor(moraDark.focus), QColor(moraDark.page)) >= 3.0);
    assert(ventu::ui_ux::contrastRatio(QColor(moraLight.focus), QColor(moraLight.page)) >= 3.0);
    return 0;
}
