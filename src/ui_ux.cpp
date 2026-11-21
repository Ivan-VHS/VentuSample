#include "ui_ux.hpp"

#include <QAbstractButton>
#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QWidget>

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
    if (widget && widget->accessibleName().trimmed().isEmpty()
        && !name.trimmed().isEmpty()) {
        widget->setAccessibleName(name.trimmed());
    }
}

} // namespace

double contrastRatio(const QColor& foreground, const QColor& background) {
    if (!foreground.isValid() || !background.isValid()) return 0.0;
    const double first = luminance(foreground);
    const double second = luminance(background);
    const double lighter = std::max(first, second);
    const double darker = std::min(first, second);
    return (lighter + 0.05) / (darker + 0.05);
}

bool meetsTextContrast(const QColor& foreground, const QColor& background,
                       bool largeText) {
    return contrastRatio(foreground, background) >= (largeText ? 3.0 : 4.5);
}

void applyAccessibleDefaults(QWidget* root) {
    if (!root) return;

    // Label buddies provide a stable accessible name for adjacent native controls.
    const auto labels = root->findChildren<QLabel*>();
    for (auto* label : labels) {
        if (label && label->buddy()) {
            setNameIfMissing(label->buddy(), label->text());
        }
    }

    const auto widgets = root->findChildren<QWidget*>();
    for (auto* widget : widgets) {
        if (!widget) continue;

        if (auto* button = qobject_cast<QAbstractButton*>(widget)) {
            setNameIfMissing(button, button->text());
            if (button->minimumHeight() < 44) button->setMinimumHeight(44);
            if (button->toolTip().isEmpty() && !button->text().isEmpty())
                button->setToolTip(button->text());
        } else if (auto* edit = qobject_cast<QLineEdit*>(widget)) {
            setNameIfMissing(edit, edit->placeholderText());
            if (edit->minimumHeight() < 40) edit->setMinimumHeight(40);
        } else if (auto* combo = qobject_cast<QComboBox*>(widget)) {
            setNameIfMissing(combo, combo->currentText().isEmpty()
                ? QStringLiteral("Выбор параметра")
                : QStringLiteral("Выбор: %1").arg(combo->currentText()));
            if (combo->minimumHeight() < 40) combo->setMinimumHeight(40);
        } else if (auto* spin = qobject_cast<QAbstractSpinBox*>(widget)) {
            setNameIfMissing(spin, QStringLiteral("Числовой параметр"));
            if (spin->minimumHeight() < 40) spin->setMinimumHeight(40);
        } else if (auto* slider = qobject_cast<QAbstractSlider*>(widget)) {
            setNameIfMissing(slider, QStringLiteral("Параметр"));
            if (slider->minimumHeight() < 40) slider->setMinimumHeight(40);
        } else if (auto* progress = qobject_cast<QProgressBar*>(widget)) {
            setNameIfMissing(progress, QStringLiteral("Ход выполнения операции"));
        }
    }
}

} // namespace ventu::ui_ux
