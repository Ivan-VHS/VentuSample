#include "preset_store.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>
#include <QStringList>

#include <cmath>
#include <limits>

namespace ventu::presets {
namespace {

constexpr qint64 MaxPresetBytes = 1024 * 1024;

const QStringList& styles() {
    static const QStringList values{
        "Boom Bap", "East Coast", "Harlem Dip-Trap", "G-Unit", "Modern Trap",
        "Atlanta Trap", "UK Drill", "UK Grime", "Memphis", "Jazz Rap", "G-Funk",
        "Boom Trap", "Cripple Trill", "Billionaire Golden Era"
    };
    return values;
}

const QStringList& targets() {
    static const QStringList values{"Full Loop", "Melody", "Drums", "Bass", "One-Shot Kit"};
    return values;
}

const QStringList& scales() {
    static const QStringList values{
        "Natural Minor", "Harmonic Minor", "Dorian", "Phrygian", "Major", "Minor Pentatonic"
    };
    return values;
}

const QStringList& samplers() {
    static const QStringList values{"Clean", "MPC 2500", "MPC 3000XL", "MPC 5000"};
    return values;
}

const QStringList& timbres() {
    static const QStringList values{"Keys", "Bell", "Pluck", "Organ", "Brass", "Strings"};
    return values;
}

bool readEnum(const QJsonObject& object, const char* key, const QStringList& choices,
              int& result, QString& error) {
    const auto value = object.value(QLatin1String(key));
    if (!value.isString()) {
        error = QStringLiteral("Preset field '%1' must be a string.").arg(QLatin1String(key));
        return false;
    }
    const int index = choices.indexOf(value.toString(), Qt::CaseSensitive);
    if (index < 0) {
        error = QStringLiteral("Preset field '%1' has an unknown value: %2")
                    .arg(QLatin1String(key), value.toString());
        return false;
    }
    result = index;
    return true;
}

bool readInteger(const QJsonObject& object, const char* key, int& result,
                 int minimum, int maximum, QString& error, bool optional = false) {
    const auto value = object.value(QLatin1String(key));
    if (optional && value.isUndefined()) return true;
    if (!value.isDouble()) {
        error = QStringLiteral("Preset field '%1' must be an integer.").arg(QLatin1String(key));
        return false;
    }
    const double number = value.toDouble();
    if (!std::isfinite(number) || std::floor(number) != number
        || number < static_cast<double>(minimum) || number > static_cast<double>(maximum)) {
        error = QStringLiteral("Preset field '%1' is outside its allowed range.").arg(QLatin1String(key));
        return false;
    }
    result = static_cast<int>(number);
    return true;
}

bool readDecimal(const QJsonObject& object, const char* key, double& result,
                 double minimum, double maximum, QString& error) {
    const auto value = object.value(QLatin1String(key));
    if (!value.isDouble()) {
        error = QStringLiteral("Preset field '%1' must be numeric.").arg(QLatin1String(key));
        return false;
    }
    const double number = value.toDouble();
    if (!std::isfinite(number) || number < minimum || number > maximum) {
        error = QStringLiteral("Preset field '%1' is outside its allowed range.").arg(QLatin1String(key));
        return false;
    }
    result = number;
    return true;
}

bool readBoolean(const QJsonObject& object, const char* key, bool& result, QString& error) {
    const auto value = object.value(QLatin1String(key));
    if (!value.isBool()) {
        error = QStringLiteral("Preset field '%1' must be true or false.").arg(QLatin1String(key));
        return false;
    }
    result = value.toBool();
    return true;
}

bool validName(const QString& name, QString& error) {
    const QString clean = name.trimmed();
    if (clean.isEmpty() || clean.size() > 80 || clean.contains(QChar::Null)) {
        error = QStringLiteral("Preset name must contain 1–80 visible characters.");
        return false;
    }
    return true;
}

bool knownFieldsOnly(const QJsonObject& object, const QStringList& allowed, QString& error) {
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (!allowed.contains(it.key())) {
            error = QStringLiteral("Unsupported field in preset v1: %1").arg(it.key());
            return false;
        }
    }
    return true;
}

} // namespace

QJsonObject toJson(const Parameters& p, const QString& name, bool includeSeed) {
    QJsonObject values{
        {"style", QString::fromStdString(ventu::toString(p.style))},
        {"target", QString::fromStdString(ventu::toString(p.target))},
        {"scale", QString::fromStdString(ventu::toString(p.scale))},
        {"sampler", QString::fromStdString(ventu::toString(p.sampler))},
        {"timbre", QString::fromStdString(ventu::toString(p.timbre))},
        {"bpm", p.bpm},
        {"root_note", p.rootNote},
        {"bars", p.bars},
        {"variations", p.variations},
        {"swing_percent", p.swingPercent},
        {"density_percent", p.densityPercent},
        {"grit_percent", p.gritPercent},
        {"saturation_percent", p.saturationPercent},
        {"stereo_percent", p.stereoPercent},
        {"high_pass_hz", p.highPassHz},
        {"low_pass_hz", p.lowPassHz},
        {"snare_offset_ms", p.snareOffsetMs},
        {"sample_rate", p.sampleRate},
        {"source_mix_percent", p.sourceMixPercent},
        {"source_pitch_semitones", p.sourcePitchSemitones},
        {"bus_compression_ratio", p.busCompressionRatio},
        {"tpdf_dither", p.tpdfDither},
        {"normalize_peak", p.normalizePeak},
        {"target_peak_dbfs", p.targetPeakDbfs},
        {"triplet_rolls", p.tripletRolls},
        {"micro_humanize", p.microHumanize},
        {"vinyl_dust", p.vinylDust},
        {"stochastic_enabled", p.stochasticEnabled},
        {"stochastic_intensity_percent", p.stochasticIntensityPercent},
        {"stochastic_trigger_mode", p.stochasticTriggerMode},
        {"stochastic_plugin_mask", static_cast<int>(p.stochasticPluginMask)},
        {"stochastic_topology", p.stochasticTopology}
    };
    if (includeSeed) values.insert("seed", p.seed);
    return QJsonObject{
        {"schema", QLatin1String(SchemaId)},
        {"name", name.trimmed()},
        {"seed_policy", includeSeed ? "fixed" : "keep-current"},
        {"parameters", values}
    };
}

bool fromJson(const QJsonObject& document, const Parameters& current,
              Parameters& restored, QString& name, QString& error) {
    error.clear();
    if (document.value("schema").toString() != QLatin1String(SchemaId)) {
        error = QStringLiteral("Unsupported preset schema. Expected %1.").arg(QLatin1String(SchemaId));
        return false;
    }
    if (!document.value("name").isString()) {
        error = QStringLiteral("Preset name must be a string.");
        return false;
    }
    name = document.value("name").toString().trimmed();
    if (!validName(name, error)) return false;

    const auto parameterValue = document.value("parameters");
    if (!parameterValue.isObject()) {
        error = QStringLiteral("Preset parameters must be a JSON object.");
        return false;
    }
    const QJsonObject values = parameterValue.toObject();
    const QStringList allowed{
        "style","target","scale","sampler","timbre","bpm","root_note","bars","variations",
        "seed","swing_percent","density_percent","grit_percent","saturation_percent","stereo_percent",
        "high_pass_hz","low_pass_hz","snare_offset_ms","sample_rate","source_mix_percent",
        "source_pitch_semitones","bus_compression_ratio","tpdf_dither","normalize_peak",
        "target_peak_dbfs","triplet_rolls","micro_humanize","vinyl_dust","stochastic_enabled",
        "stochastic_intensity_percent","stochastic_trigger_mode","stochastic_plugin_mask","stochastic_topology"
    };
    if (!knownFieldsOnly(values, allowed, error)) return false;

    const QString seedPolicy = document.value("seed_policy").toString();
    const bool hasSeed = values.contains("seed");
    if ((seedPolicy == "fixed") != hasSeed || (seedPolicy != "fixed" && seedPolicy != "keep-current")) {
        error = QStringLiteral("seed_policy must be 'fixed' when seed is present and 'keep-current' otherwise.");
        return false;
    }

    Parameters candidate = current;
    int value = 0;
    if (!readEnum(values, "style", styles(), value, error)) return false;
    candidate.style = static_cast<Style>(value);
    if (!readEnum(values, "target", targets(), value, error)) return false;
    candidate.target = static_cast<Target>(value);
    if (!readEnum(values, "scale", scales(), value, error)) return false;
    candidate.scale = static_cast<Scale>(value);
    if (!readEnum(values, "sampler", samplers(), value, error)) return false;
    candidate.sampler = static_cast<Sampler>(value);
    if (!readEnum(values, "timbre", timbres(), value, error)) return false;
    candidate.timbre = static_cast<Timbre>(value);

    if (!readInteger(values, "bpm", candidate.bpm, 60, 190, error)
        || !readInteger(values, "root_note", candidate.rootNote, 0, 11, error)
        || !readInteger(values, "bars", candidate.bars, 1, 16, error)
        || !readInteger(values, "variations", candidate.variations, 1, 32, error)
        || !readInteger(values, "swing_percent", candidate.swingPercent, 50, 68, error)
        || !readInteger(values, "density_percent", candidate.densityPercent, 0, 100, error)
        || !readInteger(values, "grit_percent", candidate.gritPercent, 0, 100, error)
        || !readInteger(values, "saturation_percent", candidate.saturationPercent, 0, 100, error)
        || !readInteger(values, "stereo_percent", candidate.stereoPercent, 0, 100, error)
        || !readInteger(values, "high_pass_hz", candidate.highPassHz, 20, 300, error)
        || !readInteger(values, "low_pass_hz", candidate.lowPassHz, 2000, 20000, error)
        || !readInteger(values, "snare_offset_ms", candidate.snareOffsetMs, -30, 30, error)
        || !readInteger(values, "sample_rate", candidate.sampleRate, 22050, 96000, error)
        || !readInteger(values, "source_mix_percent", candidate.sourceMixPercent, 0, 100, error)
        || !readInteger(values, "source_pitch_semitones", candidate.sourcePitchSemitones, -12, 12, error)
        || !readInteger(values, "stochastic_intensity_percent", candidate.stochasticIntensityPercent, 0, 100, error)
        || !readInteger(values, "stochastic_trigger_mode", candidate.stochasticTriggerMode, 0, 2, error)
        || !readInteger(values, "stochastic_plugin_mask", value, 0, 255, error)
        || !readInteger(values, "stochastic_topology", candidate.stochasticTopology, 0, 2, error)) return false;
    candidate.stochasticPluginMask = static_cast<std::uint32_t>(value);
    if (hasSeed && !readInteger(values, "seed", candidate.seed, 0, std::numeric_limits<int>::max(), error))
        return false;

    if (!readDecimal(values, "bus_compression_ratio", candidate.busCompressionRatio, 1.0, 12.0, error)
        || !readDecimal(values, "target_peak_dbfs", candidate.targetPeakDbfs, -24.0, -0.3, error)
        || !readBoolean(values, "tpdf_dither", candidate.tpdfDither, error)
        || !readBoolean(values, "normalize_peak", candidate.normalizePeak, error)
        || !readBoolean(values, "triplet_rolls", candidate.tripletRolls, error)
        || !readBoolean(values, "micro_humanize", candidate.microHumanize, error)
        || !readBoolean(values, "vinyl_dust", candidate.vinylDust, error)
        || !readBoolean(values, "stochastic_enabled", candidate.stochasticEnabled, error)) return false;

    const std::string issue = ventu::validate(candidate);
    if (!issue.empty()) {
        error = QString::fromUtf8(issue.c_str());
        return false;
    }
    restored = std::move(candidate);
    return true;
}

bool saveToFile(const QString& path, const QString& name, const Parameters& parameters,
                bool includeSeed, QString& error) {
    error.clear();
    if (!validName(name, error)) return false;
    const std::string issue = ventu::validate(parameters);
    if (!issue.empty()) {
        error = QString::fromUtf8(issue.c_str());
        return false;
    }
    if (path.trimmed().isEmpty()) {
        error = QStringLiteral("Preset file path is empty.");
        return false;
    }
    const QFileInfo info(path);
    if (!QDir().mkpath(info.absolutePath())) {
        error = QStringLiteral("Cannot create the preset directory.");
        return false;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        error = file.errorString();
        return false;
    }
    const QByteArray bytes = QJsonDocument(toJson(parameters, name, includeSeed))
                                 .toJson(QJsonDocument::Indented);
    if (bytes.size() > MaxPresetBytes || file.write(bytes) != bytes.size()) {
        error = bytes.size() > MaxPresetBytes
            ? QStringLiteral("Preset exceeds the 1 MiB file limit.")
            : file.errorString();
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        error = file.errorString();
        return false;
    }
    return true;
}

bool loadFromFile(const QString& path, const Parameters& current,
                  Parameters& restored, QString& name, QString& error) {
    error.clear();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        error = file.errorString();
        return false;
    }
    if (file.size() > MaxPresetBytes) {
        error = QStringLiteral("Preset exceeds the 1 MiB file limit.");
        return false;
    }
    const QByteArray bytes = file.read(MaxPresetBytes + 1);
    if (bytes.size() > MaxPresetBytes || file.error() != QFileDevice::NoError) {
        error = QStringLiteral("Could not read the preset within the file limit.");
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        error = QStringLiteral("Invalid preset JSON: %1").arg(parseError.errorString());
        return false;
    }
    return fromJson(document.object(), current, restored, name, error);
}

} // namespace ventu::presets
