#include "ui_motion.hpp"

#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QElapsedTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>

#include <algorithm>
#include <cmath>
#include <memory>

namespace ventu::ui_motion {

int boundedDuration(const Preferences& preferences, int requestedMs) {
    const int safeDuration = std::clamp(requestedMs, 0, 380);
    if (preferences.reduceMotion || preferences.ecoVisuals) {
        return std::min(safeDuration, 80);
    }
    if (preferences.expressiveMotion) {
        return std::min(380, static_cast<int>(std::lround(safeDuration * 1.35)));
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

static void stopPreviousSpring(QWidget* widget) {
    const auto previous = widget->findChildren<QTimer*>(QStringLiteral("ventu.motion.spring"));
    for (auto* timer : previous) {
        timer->stop();
        timer->deleteLater();
    }
}

static void animateSpring(QWidget* widget, QGraphicsOpacityEffect* effect,
                          const Preferences& preferences, int durationMs,
                          qreal startOpacity) {
    stopPreviousSpring(widget);

    auto* timer = new QTimer(widget);
    timer->setObjectName(QStringLiteral("ventu.motion.spring"));
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(16);

    auto clock = std::make_shared<QElapsedTimer>();
    clock->start();
    auto previousNs = std::make_shared<qint64>(clock->nsecsElapsed());

    qreal value = std::clamp(startOpacity, 0.0, 1.0);
    qreal velocity = 0.0;
    const qreal stiffness = preferences.expressiveMotion ? 340.0 : 240.0;
    const qreal damping = preferences.expressiveMotion ? 20.0 : 22.0;
    const qreal mass = preferences.expressiveMotion ? 0.85 : 1.0;
    const int boundedMs = boundedDuration(preferences, durationMs);
    int elapsedMs = 0;
    effect->setOpacity(value);

    QObject::connect(timer, &QTimer::timeout, widget,
        [timer, effect, clock, previousNs, value, velocity, stiffness, damping,
         mass, boundedMs, elapsedMs]() mutable {
            const qint64 nowNs = clock->nsecsElapsed();
            const qreal rawDt = static_cast<qreal>(nowNs - *previousNs) / 1.0e9;
            *previousNs = nowNs;
            const qreal dt = std::clamp(rawDt, 0.001, 0.032);
            const qreal h = dt / 2.0;

            for (int step = 0; step < 2; ++step) {
                const qreal acceleration =
                    (-stiffness * (value - 1.0) - damping * velocity) / mass;
                velocity += acceleration * h;
                value += velocity * h;
            }
            value = std::clamp(value, 0.0, 1.08);
            elapsedMs += static_cast<int>(std::lround(dt * 1000.0));
            effect->setOpacity(std::clamp(value, 0.0, 1.0));

            const bool settled = std::abs(value - 1.0) < 0.001
                && std::abs(velocity) < 0.001;
            if (settled || elapsedMs >= boundedMs) {
                effect->setOpacity(1.0);
                timer->stop();
                timer->deleteLater();
            }
        });
    timer->start();
}

static void animateOpacity(QWidget* widget, const Preferences& preferences,
                           int durationMs, qreal startOpacity) {
    const int duration = boundedDuration(preferences, durationMs);
    if (!widget || !widget->isVisible() || duration <= 0) return;
    auto* effect = opacityEffect(widget);
    if (!effect) return;

    if (!preferences.reduceMotion && !preferences.ecoVisuals) {
        animateSpring(widget, effect, preferences, duration, startOpacity);
        return;
    }

    stopPreviousSpring(widget);
    auto* animation = new QPropertyAnimation(effect, "opacity", widget);
    animation->setObjectName(QStringLiteral("ventu.motion.reduced"));
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
