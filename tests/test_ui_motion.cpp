#include "ui_motion.hpp"

#include <QCoreApplication>

#include <cassert>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    const ventu::ui_motion::Preferences normal{};
    assert(ventu::ui_motion::boundedDuration(normal, -1) == 0);
    assert(ventu::ui_motion::boundedDuration(normal, 0) == 0);
    assert(ventu::ui_motion::boundedDuration(normal, 180) == 180);
    assert(ventu::ui_motion::boundedDuration(normal, 900) == 380);

    auto expressive = normal;
    expressive.expressiveMotion = true;
    assert(ventu::ui_motion::boundedDuration(expressive, 1) == 1);
    assert(ventu::ui_motion::boundedDuration(expressive, 180) == 243);
    assert(ventu::ui_motion::boundedDuration(expressive, 380) == 380);

    auto reduced = expressive;
    reduced.reduceMotion = true;
    assert(ventu::ui_motion::boundedDuration(reduced, 380) == 80);

    auto eco = expressive;
    eco.ecoVisuals = true;
    assert(ventu::ui_motion::boundedDuration(eco, 380) == 80);

    auto reducedNormal = normal;
    reducedNormal.reduceMotion = true;
    assert(ventu::ui_motion::boundedDuration(reducedNormal, 380) == 80);

    return 0;
}
