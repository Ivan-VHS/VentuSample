#include "cineforge.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <limits>
#include <numbers>
#include <sstream>
#include <string_view>
#include <system_error>
#include <utility>

namespace ventu {
namespace {

constexpr double kPi = std::numbers::pi_v<double>;
constexpr std::size_t kMaxFrames = 1'000'000u;

struct StereoLayer {
    std::vector<float> left;
    std::vector<float> right;
};

struct Family {
    const char* name;
    double frequency;
    double duration;
    bool rhythmic;
};

constexpr std::array<Family, 10> kFamilies{{
    {"SubBoom", 48.0, 2.6, false},
    {"TitanImpact", 92.0, 2.2, false},
    {"BrassPulse", 146.0, 2.8, false},
    {"MetalHit", 410.0, 2.0, false},
    {"Ostinato", 110.0, 4.0, true},
    {"PercussionLoop", 180.0, 4.0, true},
    {"Riser", 260.0, 3.4, false},
    {"Atmosphere", 72.0, 3.8, false},
    {"HybridAccent", 210.0, 2.5, false},
    {"ImpactTail", 64.0, 3.2, false}
}};

constexpr std::array<const char*, 12> kKeys{{
    "Cmin", "Csmin", "Dmin", "Dsmin", "Emin", "Fmin",
    "Fsmin", "Gmin", "Gsmin", "Amin", "Asmin", "Bmin"
}};

std::uint32_t nextRandom(std::uint32_t& state) {
    state ^= state << 13u;
    state ^= state >> 17u;
    state ^= state << 5u;
    return state;
}

float randomSigned(std::uint32_t& state) {
    const auto value = nextRandom(state) >> 8u;
    return static_cast<float>(value) / 8388607.5f - 1.0f;
}

std::string jsonEscape(std::string_view value) {
    std::string out;
    out.reserve(value.size() + 8u);
    for (const unsigned char c : value) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 0x20u) {
                constexpr char hex[] = "0123456789abcdef";
                out += "\\u00";
                out += hex[(c >> 4u) & 0x0fu];
                out += hex[c & 0x0fu];
            } else {
                out += static_cast<char>(c);
            }
        }
    }
    return out;
}

void writeU16(std::ostream& out, std::uint16_t value) {
    out.put(static_cast<char>(value & 0xffu));
    out.put(static_cast<char>((value >> 8u) & 0xffu));
}
void writeU32(std::ostream& out, std::uint32_t value) {
    for (unsigned shift = 0; shift < 32u; shift += 8u)
        out.put(static_cast<char>((value >> shift) & 0xffu));
}

bool writeWave(const std::filesystem::path& path, const StereoLayer& audio,
               int sampleRate, CineForgeEncoding encoding, std::string& error) {
    if (audio.left.empty() || audio.left.size() != audio.right.size()
        || audio.left.size() > kMaxFrames) {
        error = "CINEFORGE refused an invalid or oversized audio buffer.";
        return false;
    }

    const std::uint16_t bits = encoding == CineForgeEncoding::Pcm24 ? 24u : 32u;
    const std::uint16_t format = encoding == CineForgeEncoding::Pcm24 ? 1u : 3u;
    const std::uint16_t blockAlign = static_cast<std::uint16_t>(2u * (bits / 8u));
    const std::uint64_t dataBytes64 =
        static_cast<std::uint64_t>(audio.left.size()) * blockAlign;
    if (dataBytes64 > std::numeric_limits<std::uint32_t>::max() - 36u) {
        error = "WAV export exceeds the RIFF 32-bit size limit.";
        return false;
    }

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        error = "Could not create WAV file: " + path.string();
        return false;
    }

    out.write("RIFF", 4);
    writeU32(out, static_cast<std::uint32_t>(36u + dataBytes64));
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    writeU32(out, 16u);
    writeU16(out, format);
    writeU16(out, 2u);
    writeU32(out, static_cast<std::uint32_t>(sampleRate));
    writeU32(out, static_cast<std::uint32_t>(sampleRate) * blockAlign);
    writeU16(out, blockAlign);
    writeU16(out, bits);
    out.write("data", 4);
    writeU32(out, static_cast<std::uint32_t>(dataBytes64));

    const auto writeSample = [&out, encoding](float value) {
        const float sample = std::clamp(std::isfinite(value) ? value : 0.0f, -0.999999f, 0.999999f);
        if (encoding == CineForgeEncoding::Pcm24) {
            const auto signedValue = static_cast<std::int32_t>(std::lrint(sample * 8388607.0f));
            const auto packed = static_cast<std::uint32_t>(signedValue);
            out.put(static_cast<char>(packed & 0xffu));
            out.put(static_cast<char>((packed >> 8u) & 0xffu));
            out.put(static_cast<char>((packed >> 16u) & 0xffu));
        } else {
            writeU32(out, std::bit_cast<std::uint32_t>(sample));
        }
    };

    for (std::size_t i = 0; i < audio.left.size(); ++i) {
        writeSample(audio.left[i]);
        writeSample(audio.right[i]);
    }
    if (!out) {
        error = "A write error occurred while exporting: " + path.string();
        return false;
    }
    return true;
}

void normalizeLayer(StereoLayer& layer, float targetPeak = 0.68f) {
    float peak = 0.0f;
    for (std::size_t i = 0; i < layer.left.size(); ++i) {
        peak = std::max(peak, std::abs(layer.left[i]));
        peak = std::max(peak, std::abs(layer.right[i]));
    }
    if (peak > 1.0e-9f) {
        const float gain = targetPeak / peak;
        for (std::size_t i = 0; i < layer.left.size(); ++i) {
            layer.left[i] *= gain;
            layer.right[i] *= gain;
        }
    }
}

StereoLayer makeLayer(std::size_t frames, int sampleRate, const Family& family,
                      std::uint64_t seed, int assetIndex, int layerKind,
                      int bpm, bool rhythmic) {
    StereoLayer layer;
    layer.left.resize(frames);
    layer.right.resize(frames);
    std::uint32_t rng = static_cast<std::uint32_t>(seed)
        ^ static_cast<std::uint32_t>(seed >> 32u)
        ^ static_cast<std::uint32_t>((assetIndex + 1) * 0x45d9f3bu)
        ^ static_cast<std::uint32_t>((layerKind + 1) * 0x119de1f3u);
    if (rng == 0u) rng = 0x6d2b79f5u;

    const double sr = static_cast<double>(sampleRate);
    const double duration = static_cast<double>(frames) / sr;
    const double beatSeconds = 60.0 / static_cast<double>(bpm);
    const double base = family.frequency * (1.0 + 0.035 * static_cast<double>(assetIndex % 5));
    double phase = 0.0;
    float noiseStateL = 0.0f;
    float noiseStateR = 0.0f;

    for (std::size_t i = 0; i < frames; ++i) {
        const double t = static_cast<double>(i) / sr;
        const double x = duration > 0.0 ? t / duration : 0.0;
        const double attack = std::exp(-t * (layerKind == 0 ? 46.0 : 15.0));
        const double bodyEnvelope = std::exp(-t * (rhythmic ? 1.7 : 2.8));
        const double tailEnvelope = std::exp(-t * (rhythmic ? 1.05 : 1.55));
        const double sweep = 1.0 + (layerKind == 2 ? 5.5 * x : 0.28 * std::sin(2.0 * kPi * 0.3 * t));
        const double frequency = base * sweep;
        phase += 2.0 * kPi * frequency / sr;
        if (phase > 2.0 * kPi) phase = std::fmod(phase, 2.0 * kPi);

        const float whiteL = randomSigned(rng);
        const float whiteR = randomSigned(rng);
        noiseStateL += 0.06f * (whiteL - noiseStateL);
        noiseStateR += 0.06f * (whiteR - noiseStateR);
        const float noiseL = noiseStateL * 0.7f + whiteL * 0.3f;
        const float noiseR = noiseStateR * 0.7f + whiteR * 0.3f;

        double gate = 1.0;
        if (rhythmic) {
            const double beatPosition = std::fmod(t, beatSeconds);
            gate = 0.18 + 0.82 * std::exp(-beatPosition * 13.0);
            if (assetIndex % 2 == 1 && std::fmod(t, beatSeconds * 2.0) > beatSeconds)
                gate *= 0.70;
        }

        double left = 0.0, right = 0.0;
        if (layerKind == 0) {
            const double click = std::exp(-t * 115.0);
            left = 0.65 * noiseL * click + 0.28 * std::sin(phase * 1.9) * attack;
            right = 0.65 * noiseR * click + 0.28 * std::sin(phase * 1.91) * attack;
        } else if (layerKind == 1) {
            left = (0.76 * std::sin(phase) + 0.18 * std::sin(phase * 2.01)
                    + 0.07 * std::sin(phase * 3.02)) * bodyEnvelope * gate;
            right = (0.74 * std::sin(phase + 0.025) + 0.19 * std::sin(phase * 2.015)
                     + 0.07 * std::sin(phase * 3.01)) * bodyEnvelope * gate;
        } else {
            const double swell = std::string_view(family.name) == "Riser" ? x : 1.0;
            left = (0.38 * noiseL + 0.14 * std::sin(phase * 0.5)) * tailEnvelope * swell;
            right = (0.38 * noiseR + 0.14 * std::sin(phase * 0.503 + 0.04))
                    * tailEnvelope * swell;
        }
        layer.left[i] = static_cast<float>(left);
        layer.right[i] = static_cast<float>(right);
    }
    return layer;
}

StereoLayer sumLayers(const StereoLayer& a, const StereoLayer& b, const StereoLayer& c) {
    StereoLayer result;
    const auto frames = a.left.size();
    result.left.resize(frames);
    result.right.resize(frames);
    for (std::size_t i = 0; i < frames; ++i) {
        result.left[i] = a.left[i] * 0.72f + b.left[i] * 0.88f + c.left[i] * 0.62f;
        result.right[i] = a.right[i] * 0.72f + b.right[i] * 0.88f + c.right[i] * 0.62f;
    }
    normalizeLayer(result, 0.72f);
    return result;
}

StereoLayer makeWet(const StereoLayer& dry, int sampleRate) {
    StereoLayer wet;
    wet.left.resize(dry.left.size());
    wet.right.resize(dry.right.size());
    const std::size_t tapA = std::max<std::size_t>(1u, static_cast<std::size_t>(sampleRate * 0.031));
    const std::size_t tapB = std::max<std::size_t>(1u, static_cast<std::size_t>(sampleRate * 0.067));
    const std::size_t tapC = std::max<std::size_t>(1u, static_cast<std::size_t>(sampleRate * 0.113));
    for (std::size_t i = 0; i < dry.left.size(); ++i) {
        const auto delayed = [i](const std::vector<float>& v, std::size_t tap) {
            return i >= tap ? v[i - tap] : 0.0f;
        };
        wet.left[i] = dry.left[i] + 0.25f * delayed(dry.right, tapA)
                    + 0.13f * delayed(dry.left, tapB) + 0.07f * delayed(dry.right, tapC);
        wet.right[i] = dry.right[i] + 0.25f * delayed(dry.left, tapA)
                     + 0.13f * delayed(dry.right, tapB) + 0.07f * delayed(dry.left, tapC);
    }
    normalizeLayer(wet, 0.78f);
    return wet;
}

bool validSettings(const CineForgeSettings& s, std::string& error) {
    if (s.scenePrompt.empty() || s.scenePrompt.size() > 512u) {
        error = "Scene prompt must contain 1..512 UTF-8 bytes."; return false;
    }
    if (s.bpm < 60 || s.bpm > 190) { error = "BPM must be within 60..190."; return false; }
    if (s.rootNote < 0 || s.rootNote >= 12) { error = "Root note must be within 0..11."; return false; }
    if (s.bars < 1 || s.bars > 16) { error = "Bars must be within 1..16."; return false; }
    if (s.packSize < 8 || s.packSize > 12) { error = "Pack size must be within 8..12 assets."; return false; }
    if (s.sampleRate != 48000 && s.sampleRate != 96000) {
        error = "CINEFORGE supports 48 kHz and 96 kHz."; return false;
    }
    if (s.outputDirectory.empty()) { error = "Choose a local output directory."; return false; }
    return true;
}

const char* encodingName(CineForgeEncoding encoding) {
    return encoding == CineForgeEncoding::Pcm24 ? "PCM 24-bit" : "IEEE Float 32-bit";
}

StereoLayer renderFamily(const Family& family, std::size_t frames, int sampleRate,
                         std::uint64_t seed, int index, int bpm) {
    auto a = makeLayer(frames, sampleRate, family, seed, index, 0, bpm, family.rhythmic);
    auto b = makeLayer(frames, sampleRate, family, seed, index, 1, bpm, family.rhythmic);
    auto c = makeLayer(frames, sampleRate, family, seed, index, 2, bpm, family.rhythmic);
    normalizeLayer(a, 0.50f); normalizeLayer(b, 0.56f); normalizeLayer(c, 0.38f);
    return sumLayers(a, b, c);
}

} // namespace

bool generateCineForgePack(const CineForgeSettings& s, CineForgeResult& result) {
    result = {};
    if (!validSettings(s, result.error)) return false;

    const std::string key = kKeys[static_cast<std::size_t>(s.rootNote)];
    std::ostringstream dirname;
    dirname << "CINEFORGE_" << s.bpm << "bpm_" << key << "_seed" << s.seed;
    result.packDirectory = s.outputDirectory / dirname.str();

    std::error_code ec;
    if (!std::filesystem::create_directories(result.packDirectory, ec) || ec) {
        result.error = "Could not create a new pack folder (existing folders are never overwritten): "
            + result.packDirectory.string();
        result.packDirectory.clear();
        return false;
    }

    result.assets.reserve(static_cast<std::size_t>(s.packSize));
    std::ostringstream manifest;
    manifest << "{\n"
             << "  \"schema\": \"cineforge.pack.v1\",\n"
             << "  \"engine\": \"Ventu CINEFORGE procedural scene-pack renderer\",\n"
             << "  \"scene_prompt\": \"" << jsonEscape(s.scenePrompt) << "\",\n"
             << "  \"bpm\": " << s.bpm << ",\n"
             << "  \"musical_key\": \"" << key << "\",\n"
             << "  \"bars\": " << s.bars << ",\n"
             << "  \"seed\": " << s.seed << ",\n"
             << "  \"sample_rate\": " << s.sampleRate << ",\n"
             << "  \"encoding\": \"" << encodingName(s.encoding) << "\",\n"
             << "  \"render_model\": \"seeded procedural synthesis; prompt stored as metadata\",\n"
             << "  \"assets\": [\n";

    for (int index = 0; index < s.packSize; ++index) {
        const auto& family = kFamilies[static_cast<std::size_t>(index) % kFamilies.size()];
        const double requestedDuration = family.rhythmic
            ? std::min(8.0, static_cast<double>(s.bars * 4) * 60.0 / s.bpm)
            : family.duration;
        const auto frames = std::clamp<std::size_t>(
            static_cast<std::size_t>(std::ceil(requestedDuration * s.sampleRate)), 256u, kMaxFrames);
        const auto seed = s.seed + static_cast<std::uint64_t>(index + 1)
            * 0x9e3779b97f4a7c15ULL;

        auto attack = makeLayer(frames, s.sampleRate, family, seed, index, 0, s.bpm, family.rhythmic);
        auto body = makeLayer(frames, s.sampleRate, family, seed, index, 1, s.bpm, family.rhythmic);
        auto tail = makeLayer(frames, s.sampleRate, family, seed, index, 2, s.bpm, family.rhythmic);
        normalizeLayer(attack, 0.50f);
        normalizeLayer(body, 0.56f);
        normalizeLayer(tail, 0.38f);
        auto dry = sumLayers(attack, body, tail);
        auto wet = makeWet(dry, s.sampleRate);

        std::ostringstream stem;
        stem << "CF_" << s.bpm << "bpm_" << key << "_" << family.name
             << "_" << std::setw(2) << std::setfill('0') << (index + 1);
        const std::array<std::pair<const char*, const StereoLayer*>, 5> outputs{{
            {"Dry", &dry}, {"Wet", &wet}, {"Attack", &attack}, {"Body", &body}, {"Tail", &tail}
        }};

        CineForgeAssetResult asset;
        asset.name = family.name;
        std::vector<std::string> exportedNames;
        exportedNames.reserve(outputs.size());
        for (const auto& [suffix, audio] : outputs) {
            const auto filename = stem.str() + "_" + suffix + ".wav";
            const auto path = result.packDirectory / filename;
            if (!writeWave(path, *audio, s.sampleRate, s.encoding, result.error))
                return false;
            asset.files.push_back(path);
            exportedNames.push_back(filename);
        }
        result.assets.push_back(std::move(asset));

        manifest << "    {\"name\":\"" << family.name << "\",\"seed\":" << seed
                 << ",\"frames\":" << frames << ",\"exports\":[";
        for (std::size_t file = 0; file < exportedNames.size(); ++file) {
            if (file) manifest << ",";
            manifest << "\"" << exportedNames[file] << "\"";
        }
        manifest << "]}" << (index + 1 == s.packSize ? "\n" : ",\n");
    }
    manifest << "  ],\n"
             << "  \"safety\": {\"normalized_peak_target_dbfs\": -2.1, \"stereo_wet_reflections\": true, \"source_overwrite\": false},\n"
             << "  \"limitations\": [\"Scene prompt is metadata, not neural text-to-audio input\", "
             << "\"Attack/Body/Tail are synthesis stems, not source-separation estimates\", "
             << "\"Wet is a deterministic multi-tap reflection model\"]\n"
             << "}\n";

    result.manifestPath = result.packDirectory / "cineforge_manifest.json";
    {
        std::ofstream out(result.manifestPath, std::ios::binary | std::ios::trunc);
        out << manifest.str();
        if (!out) { result.error = "Could not write CINEFORGE JSON manifest."; return false; }
    }

    std::vector<StereoLayer> previewParts;
    for (std::size_t i = 0; i < std::min<std::size_t>(3u, result.assets.size()); ++i) {
        const auto& family = kFamilies[i % kFamilies.size()];
        const double duration = family.rhythmic
            ? std::min(8.0, static_cast<double>(s.bars * 4) * 60.0 / s.bpm)
            : family.duration;
        const auto frames = std::clamp<std::size_t>(
            static_cast<std::size_t>(std::ceil(duration * s.sampleRate)), 256u, kMaxFrames);
        const auto seed = s.seed + static_cast<std::uint64_t>(i + 1u) * 0x9e3779b97f4a7c15ULL;
        previewParts.push_back(renderFamily(family, frames, s.sampleRate, seed, static_cast<int>(i), s.bpm));
    }
    std::size_t previewFrames = 0u;
    for (const auto& part : previewParts) previewFrames = std::max(previewFrames, part.left.size());
    StereoLayer preview;
    preview.left.assign(previewFrames, 0.0f);
    preview.right.assign(previewFrames, 0.0f);
    const float partGain = 1.0f / static_cast<float>(previewParts.size());
    for (const auto& part : previewParts) {
        for (std::size_t i = 0; i < part.left.size(); ++i) {
            preview.left[i] += part.left[i] * partGain;
            preview.right[i] += part.right[i] * partGain;
        }
    }
    normalizeLayer(preview, 0.72f);
    result.previewPath = result.packDirectory / "CINEFORGE_Preview.wav";
    if (!writeWave(result.previewPath, preview, s.sampleRate, s.encoding, result.error))
        return false;

    result.success = true;
    return true;
}

} // namespace ventu
