#include "ui_motion.hpp"

#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include <algorithm>

namespace ventu::ui_motion {

int boundedDuration(const Preferences& preferences, int requestedMs) {
    const int safeDuration = std::clamp(requestedMs, 0, 380);
    if (preferences.reduceMotion || preferences.ecoVisuals) {
        return std::min(safeDuration, 80);
    }
    return safeDuration;
}

static QGraphicsOpacityEffect* opacityEffect(QWidget* widget) {
    if (!widget) return nullptr;
    if (auto* existing = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect()))
        return existing;
    // Never replace an effect owned by another visual subsystem.
    if (widget->graphicsEffect()) return nullptr;
    auto* effect = new QGraphicsOpacityEffect(widget);
    effect->setOpacity(1.0);
    widget->setGraphicsEffect(effect);
    return effect;
}

static void animateOpacity(QWidget* widget, const Preferences& preferences,
                           int durationMs, qreal startOpacity) {
    const int duration = boundedDuration(preferences, durationMs);
    if (!widget || !widget->isVisible() || duration <= 0) return;
    auto* effect = opacityEffect(widget);
    if (!effect) return;

    auto* animation = new QPropertyAnimation(effect, "opacity", widget);
    animation->setDuration(duration);
    animation->setStartValue(std::clamp(startOpacity, 0.0, 1.0));
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve(QEasingCurve::OutCubic));
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void fadeIn(QWidget* widget, const Preferences& preferences, int durationMs) {
    animateOpacity(widget, preferences, durationMs, 0.94);
}

void pulse(QWidget* widget, const Preferences& preferences, int durationMs) {
    animateOpacity(widget, preferences, durationMs, 0.68);
}

} // namespace ventu::ui_motion
