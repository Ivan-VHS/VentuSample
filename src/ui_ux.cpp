#include "ui_ux.hpp"

#include <QAbstractButton>
#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QAbstractSpinBox>

#include <algorithm>
#include <cmath>

namespace ventu::ui_ux {
namespace {
double linearChannel(double value) {
    const double channel = std::clamp(value, 0.0, 1.0);
    return channel <= 0.04045
        ? channel / 12.92
        : std::pow((channel + 0.055) / 1.055, 2.4);
}

double luminance(const QColor& color) {
    return 0.2126 * linearChannel(color.redF())
         + 0.7152 * linearChannel(color.greenF())
         + 0.0722 * linearChannel(color.blueF());
}

void setNameIfMissing(QWidget* widget, const QString& name) {
    if (widget && widget->accessibleName().trimmed().isEmpty() && !name.trimmed().isEmpty())
        widget->setAccessibleName(name.trimmed());
}
} // namespace

double contrastRatio(const QColor& foreground, const QColor& background) {
    if (!foreground.isValid() || !background.isValid()) return 0.0;
    const double first = luminance(foreground);
    const double second = luminance(background);
    return (std::max(first, second) + 0.05) / (std::min(first, second) + 0.05);
}

bool meetsTextContrast(const QColor& foreground, const QColor& background, bool largeText) {
    return contrastRatio(foreground, background) >= (largeText ? 3.0 : 4.5);
}

int clampFontSize(int value) {
    return std::clamp(value, 9, 18);
}

void applyAccessibleDefaults(QWidget* root) {
    if (!root) return;
    for (auto* label : root->findChildren<QLabel*>()) {
        if (label && label->buddy())
            setNameIfMissing(label->buddy(), label->text());
    }

    for (auto* widget : root->findChildren<QWidget*>()) {
        if (!widget) continue;
        if (auto* button = qobject_cast<QAbstractButton*>(widget)) {
            setNameIfMissing(button, button->text());
            if (button->minimumHeight() < 44) button->setMinimumHeight(44);
            button->setFocusPolicy(Qt::StrongFocus);
            if (button->toolTip().isEmpty() && !button->text().isEmpty())
                button->setToolTip(button->text());
        } else if (auto* edit = qobject_cast<QLineEdit*>(widget)) {
            setNameIfMissing(edit, edit->placeholderText());
            if (edit->minimumHeight() < 44) edit->setMinimumHeight(44);
            edit->setFocusPolicy(Qt::StrongFocus);
        } else if (auto* combo = qobject_cast<QComboBox*>(widget)) {
            setNameIfMissing(combo, combo->currentText().isEmpty()
                ? QStringLiteral("Выбор параметра")
                : QStringLiteral("Выбор: %1").arg(combo->currentText()));
            if (combo->minimumHeight() < 44) combo->setMinimumHeight(44);
            combo->setFocusPolicy(Qt::StrongFocus);
        } else if (auto* spin = qobject_cast<QAbstractSpinBox*>(widget)) {
            setNameIfMissing(spin, QStringLiteral("Числовой параметр"));
            if (spin->minimumHeight() < 44) spin->setMinimumHeight(44);
            spin->setFocusPolicy(Qt::StrongFocus);
        } else if (auto* slider = qobject_cast<QAbstractSlider*>(widget)) {
            setNameIfMissing(slider, QStringLiteral("Параметр"));
            if (slider->minimumHeight() < 44) slider->setMinimumHeight(44);
            slider->setFocusPolicy(Qt::StrongFocus);
        } else if (auto* progress = qobject_cast<QProgressBar*>(widget)) {
            setNameIfMissing(progress, QStringLiteral("Ход выполнения операции"));
        }
    }
}
} // namespace ventu::ui_ux
