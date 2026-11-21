#pragma once

#include <QWidget>

namespace ventu::ui_motion {

struct Preferences {
    bool reduceMotion{false};
    bool ecoVisuals{false};
    bool syncToBpm{false};
    bool expressiveMotion{false};
};

int boundedDuration(const Preferences& preferences, int requestedMs);
void fadeIn(QWidget* widget, const Preferences& preferences, int durationMs = 180);
void pulse(QWidget* widget, const Preferences& preferences, int durationMs = 70);

} // namespace ventu::ui_motion
