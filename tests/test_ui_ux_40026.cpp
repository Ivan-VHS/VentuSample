#include "ui_ux.hpp"

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
    return 0;
}
