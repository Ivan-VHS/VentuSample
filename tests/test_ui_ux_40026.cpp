#include "ui_ux.hpp"
#include "../src/mora_theme.hpp"

#include <QColor>
#include <cassert>
#include <cmath>
#include <string>

int main() {
    const QColor white("#FFFFFF"), navy("#0A1E46"), blue("#0039A6"), grey("#808080");
    assert(ventu::ui_ux::meetsTextContrast(white, navy));
    assert(ventu::ui_ux::meetsTextContrast(white, blue));
    assert(!ventu::ui_ux::meetsTextContrast(grey, white));
    assert(ventu::ui_ux::meetsTextContrast(grey, white, true));
    assert(ventu::ui_ux::contrastRatio(QColor(), white) == 0.0);
    assert(std::isfinite(ventu::ui_ux::contrastRatio(white, navy)));

    const auto duck = mora::ui::palette();
    assert(std::string(duck.page) == "#080808");
    assert(std::string(duck.surface) == "#151515");
    assert(std::string(duck.ink) == "#FFFFFF");
    assert(std::string(duck.accent) == "#FFD400");
    assert(std::string(duck.onAccent) == "#080808");
    assert(std::string(duck.signal) == "#FFD400");
    assert(std::string(duck.onSignal) == "#080808");
    assert(ventu::ui_ux::meetsTextContrast(QColor(duck.onAccent), QColor(duck.accent)));
    assert(ventu::ui_ux::meetsTextContrast(QColor(duck.onSignal), QColor(duck.signal)));
    assert(ventu::ui_ux::contrastRatio(QColor(duck.focus), QColor(duck.page)) >= 3.0);
    return 0;
}
