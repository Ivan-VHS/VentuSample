#include "preset_store.hpp"

#include <QCoreApplication>
#include <QJsonObject>
#include <QTemporaryDir>

#include <cstdlib>
#include <iostream>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    ventu::Parameters source;
    source.style = ventu::Style::GUnit;
    source.target = ventu::Target::Drums;
    source.scale = ventu::Scale::Dorian;
    source.sampler = ventu::Sampler::MPC2500;
    source.timbre = ventu::Timbre::Brass;
    source.bpm = 150;
    source.rootNote = 9;
    source.bars = 8;
    source.variations = 4;
    source.seed = 98765;
    source.swingPercent = 62;
    source.densityPercent = 81;
    source.gritPercent = 33;
    source.saturationPercent = 46;
    source.stereoPercent = 70;
    source.highPassHz = 38;
    source.lowPassHz = 15000;
    source.sampleRate = 48000;
    source.stochasticIntensityPercent = 57;
    source.stochasticTriggerMode = 1;
    source.stochasticTopology = 2;
    source.stochasticPluginMask = 0x35u;
    source.stochasticEnabled = true;

    QString error;
    QString name;
    ventu::Parameters restored;
    restored.seed = 1234;
    const auto portable = ventu::presets::toJson(source, "Harlem 150", false);
    require(!portable.value("parameters").toObject().contains("seed"),
            "seed must be omitted by default");
    require(ventu::presets::fromJson(portable, restored, restored, name, error),
            "portable preset should load");
    require(name == "Harlem 150", "preset name should round trip");
    require(restored.seed == 1234, "portable preset should preserve the current seed");
    require(restored.bpm == source.bpm && restored.rootNote == source.rootNote,
            "musical settings should round trip");
    require(restored.style == source.style && restored.target == source.target
            && restored.scale == source.scale && restored.sampler == source.sampler,
            "enum selections should round trip");
    require(restored.stochasticPluginMask == source.stochasticPluginMask
            && restored.stochasticTopology == source.stochasticTopology,
            "stochastic routing settings should round trip");

    const auto fixed = ventu::presets::toJson(source, "Fixed seed", true);
    ventu::Parameters fixedResult;
    require(ventu::presets::fromJson(fixed, source, fixedResult, name, error),
            "fixed-seed preset should load");
    require(fixedResult.seed == source.seed, "explicit seed should round trip");

    auto broken = fixed;
    broken.insert("schema", "ventusample.preset.v99");
    require(!ventu::presets::fromJson(broken, source, fixedResult, name, error),
            "unknown schema must be rejected");

    auto invalid = portable;
    auto values = invalid.value("parameters").toObject();
    values.insert("bpm", 999);
    invalid.insert("parameters", values);
    require(!ventu::presets::fromJson(invalid, source, fixedResult, name, error),
            "out-of-range BPM must be rejected");

    QTemporaryDir directory;
    require(directory.isValid(), "temporary directory should be available");
    const QString path = directory.filePath("harlem.json");
    require(ventu::presets::saveToFile(path, "Harlem 150", source, true, error),
            "preset should save atomically");
    ventu::Parameters loaded;
    require(ventu::presets::loadFromFile(path, source, loaded, name, error),
            "saved preset should reload");
    require(loaded.seed == source.seed && loaded.bpm == source.bpm,
            "saved preset values should remain intact");

    std::cout << "VentuSample preset tests passed.\\n";
    return 0;
}
