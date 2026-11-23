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

    const auto moraAmber = mora::ui::palette(mora::ui::Theme::Amber);
    assert(QString::fromLatin1(moraAmber.page) == QStringLiteral("#211913"));
    assert(QString::fromLatin1(moraAmber.accent) == QStringLiteral("#D58B3D"));
    assert(QString::fromLatin1(moraAmber.signal) == QStringLiteral("#B64A40"));
    assert(ventu::ui_ux::contrastRatio(QColor(moraAmber.ink), QColor(moraAmber.page)) >= 7.0);
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraAmber.onAccent), QColor(moraAmber.accent)));
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraAmber.onSignal), QColor(moraAmber.signal)));
    const auto moraDark = mora::ui::palette(true);
    const auto moraLight = mora::ui::palette(false);
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraDark.onAccent), QColor(moraDark.accent)));
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraDark.onSignal), QColor(moraDark.signal)));
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraLight.onAccent), QColor(moraLight.accent)));
    assert(ventu::ui_ux::meetsTextContrast(QColor(moraLight.onSignal), QColor(moraLight.signal)));
    assert(ventu::ui_ux::contrastRatio(QColor(moraDark.focus), QColor(moraDark.page)) >= 3.0);
    assert(ventu::ui_ux::contrastRatio(QColor(moraLight.focus), QColor(moraLight.page)) >= 3.0);
    return 0;
}
