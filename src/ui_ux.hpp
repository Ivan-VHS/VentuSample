#pragma once

#include <QColor>
#include <QWidget>

namespace ventu::ui_ux {

double contrastRatio(const QColor& foreground, const QColor& background);
bool meetsTextContrast(const QColor& foreground, const QColor& background,
                       bool largeText = false);
void applyAccessibleDefaults(QWidget* root);

} // namespace ventu::ui_ux
