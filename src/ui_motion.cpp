#include "ui_motion.hpp"

#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QElapsedTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPen>
#include <QSizePolicy>
#include <QTimer>

#include <algorithm>
#include <cmath>
#include <memory>

namespace ventu::ui_motion {

DraperyBanner::DraperyBanner(QWidget* parent)
    : QWidget(parent), timer_(new QTimer(this)) {
    setObjectName(QStringLiteral("duckDraperyBanner"));
    setAccessibleName(QStringLiteral("DuckOS black white and yellow drapery accent"));
    setMinimumHeight(52);
    setMaximumHeight(52);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    timer_->setInterval(100);
    timer_->setTimerType(Qt::CoarseTimer);
    connect(timer_, &QTimer::timeout, this, [this] { advance(); });
    if (motionAllowed()) timer_->start();
}

void DraperyBanner::setMotionPreferences(const Preferences& preferences) {
    preferences_ = preferences;
    if (motionAllowed()) {
        if (!timer_->isActive()) timer_->start();
    } else {
        timer_->stop();
        phase_ = 0.0;
    }
    update();
}

bool DraperyBanner::motionAllowed() const {
    const QString override = qEnvironmentVariable("UI_REDUCED_MOTION").trimmed().toLower();
    const bool environmentReduced = override == QStringLiteral("1")
        || override == QStringLiteral("true")
        || override == QStringLiteral("yes")
        || override == QStringLiteral("on");
    return !preferences_.reduceMotion && !preferences_.ecoVisuals && !environmentReduced;
}

void DraperyBanner::advance() {
    if (!motionAllowed()) {
        timer_->stop();
        phase_ = 0.0;
        update();
        return;
    }
    phase_ = std::fmod(phase_ + 0.055, 2.0 * 3.14159265358979323846);
    update();
}

void DraperyBanner::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const qreal width = static_cast<qreal>(this->width());
    const qreal height = static_cast<qreal>(this->height());
    if (width <= 0.0 || height <= 0.0) return;

    QLinearGradient cloth(0.0, 0.0, 0.0, height);
    cloth.setColorAt(0.0, QColor(QStringLiteral("#242424")));
    cloth.setColorAt(0.18, QColor(QStringLiteral("#151515")));
    cloth.setColorAt(0.52, QColor(QStringLiteral("#080808")));
    cloth.setColorAt(0.82, QColor(QStringLiteral("#151515")));
    cloth.setColorAt(1.0, QColor(QStringLiteral("#080808")));
    painter.fillRect(rect(), cloth);

    const int foldCount = std::clamp(static_cast<int>(width / 68.0), 8, 30);
    const qreal spacing = width / static_cast<qreal>(foldCount);
    const qreal amplitude = motionAllowed()
        ? (preferences_.expressiveMotion ? 2.5 : 1.35)
        : 0.0;

    for (int index = 0; index <= foldCount; ++index) {
        const qreal baseX = static_cast<qreal>(index) * spacing
            + std::sin(static_cast<qreal>(index) * 1.73) * spacing * 0.13;
        const qreal offset = std::sin(phase_ + static_cast<qreal>(index) * 0.61) * amplitude;

        QPainterPath valley;
        valley.moveTo(baseX, -2.0);
        valley.cubicTo(baseX + offset * 0.7, height * 0.28,
                       baseX - offset * 0.55, height * 0.72,
                       baseX + offset, height + 2.0);
        painter.setPen(QPen(QColor(0, 0, 0, 190),
                            std::max(3.0, spacing * 0.16),
                            Qt::SolidLine, Qt::RoundCap));
        painter.drawPath(valley);

        QPainterPath ridge;
        const qreal ridgeX = baseX + spacing * 0.17;
        ridge.moveTo(ridgeX, -2.0);
        ridge.cubicTo(ridgeX + offset * 0.48, height * 0.3,
                      ridgeX - offset * 0.32, height * 0.72,
                      ridgeX + offset * 0.65, height + 2.0);
        painter.setPen(QPen(QColor(255, 212, 0, 135),
                            std::max(1.0, spacing * 0.035),
                            Qt::SolidLine, Qt::RoundCap));
        painter.drawPath(ridge);

        QPainterPath softLight;
        softLight.moveTo(ridgeX + spacing * 0.08, -2.0);
        softLight.cubicTo(ridgeX + spacing * 0.08, height * 0.35,
                          ridgeX + spacing * 0.08, height * 0.68,
                          ridgeX + spacing * 0.08, height + 2.0);
        painter.setPen(QPen(QColor(255, 255, 255, 48), 1.0));
        painter.drawPath(softLight);
    }

    painter.fillRect(QRect(0, 0, width, 2), QColor(255, 212, 0, 150));
    painter.fillRect(QRect(0, static_cast<int>(height) - 3, width, 3),
                     QColor(0, 0, 0, 220));
}


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
