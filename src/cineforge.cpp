#include "cineforge.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <limits>
#include <random>
#include <sstream>
#include <system_error>

namespace ventu::cineforge {
namespace {
constexpr double pi = 3.14159265358979323846;
constexpr double ceiling = 0.89;
constexpr std::array<const char*, 12> names{
    "SubBoom", "TitanImpact", "BrassPulse", "MetalHit", "Ostinato",
    "PercussionLoop", "Riser", "DeepAtmosphere", "ReverseSwell",
    "HybridThud", "HarmonicDrone", "TrailerAccent"
};
double midiHz(int note) { return 440.0 * std::pow(2.0, (note - 69.0) / 12.0); }
double clamp01(double x) { return std::clamp(x, 0.0, 1.0); }

void add(AudioBuffer& a, std::size_t frame, double value, double pan = 0.0) {
    if (frame >= a.frames()) return;
    const double p = std::clamp(pan, -1.0, 1.0);
    const auto i = frame * 2u;
    a.samples[i] += static_cast<float>(value * std::sqrt((1.0 - p) * 0.5));
    a.samples[i + 1u] += static_cast<float>(value * std::sqrt((1.0 + p) * 0.5));
}

AudioBuffer makeBuffer(int rate, double seconds) {
    AudioBuffer a;
    a.sampleRate = rate;
    a.channels = 2;
    const auto frames = static_cast<std::size_t>(std::ceil(rate * seconds));
    a.samples.assign(frames * 2u, 0.0F);
    return a;
}

void tone(AudioBuffer& a, std::size_t start, double hz, double seconds,
          double gain, int mood, std::mt19937& rng, bool metallic = false) {
    const auto count = static_cast<std::size_t>(seconds * a.sampleRate);
    const double grit = mood / 100.0;
    double phase = 0.0;
    for (std::size_t n = 0; n < count && start + n < a.frames(); ++n) {
        const double t = static_cast<double>(n) / a.sampleRate;
        const double f = hz * (metallic ? 1.0 + 0.025 * std::exp(-t * 12.0) : 1.0);
        phase += 2.0 * pi * f / a.sampleRate;
        const double env = std::min(1.0, t / 0.006) * std::exp(-t / std::max(0.08, seconds * 0.58));
        double wave = std::sin(phase) + 0.24 * std::sin(phase * (metallic ? 2.73 : 2.0));
        if (metallic) wave += 0.14 * std::sin(phase * 4.17);
        const double noise = grit * 0.025 * (static_cast<double>(rng() % 20001u) / 10000.0 - 1.0);
        const double pan = metallic ? 0.16 * std::sin(t * 8.0) : 0.0;
        add(a, start + n, std::tanh((wave + noise) * 0.72) * env * gain, pan);
    }
}

void impact(AudioBuffer& a, double root, int punch, int mood, std::mt19937& rng) {
    const double gain = 0.30 + punch * 0.004;
    double phase = 0.0;
    for (std::size_t n = 0; n < a.frames(); ++n) {
        const double t = static_cast<double>(n) / a.sampleRate;
        const double f = 34.0 + 92.0 * std::exp(-t * 17.0);
        phase += 2.0 * pi * f / a.sampleRate;
        const double body = std::sin(phase) * std::exp(-t * 4.5);
        const double transient = n < static_cast<std::size_t>(a.sampleRate * 0.012)
            ? (static_cast<double>(rng() % 20001u) / 10000.0 - 1.0) * 0.22 * std::exp(-t * 380.0) : 0.0;
        const double harmonic = 0.16 * std::sin(2.0 * pi * root * 0.5 * t) * std::exp(-t * 2.2);
        add(a, n, (body + transient + harmonic) * gain);
    }
}

void renderAsset(AudioBuffer& a, int index, const Settings& s, int variation, double root) {
    std::mt19937 rng(static_cast<std::uint32_t>(s.seed + index * 7919 + variation * 104729));
    const double mood = s.mood / 100.0;
    const double punch = s.punch / 100.0;
    const double beat = 60.0 / s.bpm;
    const double duration = static_cast<double>(a.frames()) / a.sampleRate;
    switch (index % 12) {
    case 0: impact(a, root, s.punch, s.mood, rng); break;
    case 1:
        impact(a, root * 1.5, std::min(100, s.punch + 10), s.mood, rng);
        tone(a, 0, root * 2.0, duration * 0.42, 0.12 + mood * 0.12, s.mood, rng, true);
        break;
    case 2:
        for (int i = 0; i < 4; ++i)
            tone(a, static_cast<std::size_t>(i * beat * a.sampleRate), root * (i % 2 ? 1.5 : 2.0),
                 beat * 0.82, 0.20 + punch * 0.08, s.mood, rng);
        break;
    case 3:
        for (int i = 0; i < 7; ++i)
            tone(a, static_cast<std::size_t>(i * duration * a.sampleRate / 8.0),
                 root * (2.7 + i * 0.31), 0.75, 0.17, s.mood, rng, true);
        break;
    case 4:
    case 10:
        for (int i = 0; i < 8; ++i)
            tone(a, static_cast<std::size_t>(i * beat * 0.5 * a.sampleRate),
                 root * std::array<double, 4>{1.0, 1.25, 1.5, 2.0}[i % 4],
                 beat * 0.46, 0.12 + mood * 0.08, s.mood, rng);
        break;
    case 5:
        for (int i = 0; i < 16; ++i) {
            const auto start = static_cast<std::size_t>(i * beat * 0.25 * a.sampleRate);
            const double hz = 1800.0 + (i % 4) * 320.0;
            tone(a, start, hz, 0.075, 0.13 + punch * 0.08, s.mood, rng, true);
        }
        break;
    case 6:
    case 8:
        for (std::size_t n = 0; n < a.frames(); ++n) {
            const double t = static_cast<double>(n) / a.sampleRate;
            const double rise = std::pow(clamp01(t / std::max(0.1, duration)), 2.0);
            const double noise = (static_cast<double>(rng() % 20001u) / 10000.0 - 1.0);
            add(a, n, noise * rise * (0.025 + mood * 0.10));
        }
        tone(a, 0, root, duration, 0.07, s.mood, rng, true);
        break;
    case 7:
        tone(a, 0, root * 0.5, duration, 0.18, s.mood, rng);
        tone(a, 0, root * 1.003, duration, 0.08, s.mood, rng, true);
        break;
    case 9:
    case 11:
        impact(a, root * (index == 9 ? 0.75 : 1.25), s.punch, s.mood, rng);
        tone(a, 0, root * (index == 9 ? 1.0 : 2.0), duration * 0.65, 0.13, s.mood, rng, true);
        break;
    }
    // Deterministic micro-take movement is deliberately subtle and never changes pack tonality.
    if (variation > 0) {
        const double shift = 1.0 + variation * 0.0015;
        for (std::size_t n = 0; n < a.frames(); ++n) {
            const double t = static_cast<double>(n) / a.sampleRate;
            const double modulation = 1.0 + (shift - 1.0) * std::sin(2.0 * pi * t * 0.37);
            a.samples[n * 2u] = static_cast<float>(a.samples[n * 2u] * modulation);
            a.samples[n * 2u + 1u] = static_cast<float>(a.samples[n * 2u + 1u] * modulation);
        }
    }
}

AudioBuffer wetVersion(const AudioBuffer& dry, int space) {
    AudioBuffer wet = dry;
    const double amount = space / 100.0;
    const std::array<int, 3> delays{static_cast<int>(0.031 * dry.sampleRate),
                                   static_cast<int>(0.047 * dry.sampleRate),
                                   static_cast<int>(0.071 * dry.sampleRate)};
    const std::array<double, 3> gains{0.22, 0.14, 0.08};
    for (std::size_t n = 0; n < dry.frames(); ++n) {
        for (std::size_t tap = 0; tap < delays.size(); ++tap) {
            const auto delay = static_cast<std::size_t>(delays[tap]);
            if (n < delay) continue;
            const auto src = (n - delay) * 2u;
            const auto dst = n * 2u;
            const double decay = std::exp(-static_cast<double>(n) / (dry.sampleRate * 1.9));
            wet.samples[dst] += static_cast<float>(dry.samples[src] * gains[tap] * amount * decay);
            wet.samples[dst + 1u] += static_cast<float>(dry.samples[src + 1u] * gains[tap] * amount * decay);
        }
    }
    return wet;
}

void normalize(AudioBuffer& a) {
    double peak = 0.0;
    for (float x : a.samples) peak = std::max(peak, std::abs(static_cast<double>(x)));
    if (peak <= 1.0e-9) return;
    const double gain = std::min(1.0, ceiling / peak);
    for (float& x : a.samples) x = static_cast<float>(std::clamp(static_cast<double>(x) * gain, -ceiling, ceiling));
}

void put16(std::ofstream& f, std::uint16_t x) {
    f.put(static_cast<char>(x & 0xffu)); f.put(static_cast<char>((x >> 8u) & 0xffu));
}
void put32(std::ofstream& f, std::uint32_t x) {
    for (int i = 0; i < 4; ++i) f.put(static_cast<char>((x >> (i * 8)) & 0xffu));
}
bool write24(const std::filesystem::path& path, const AudioBuffer& a, std::string& error) {
    if (a.channels != 2 || a.samples.empty() || a.samples.size() > (std::numeric_limits<std::uint32_t>::max() - 36u) / 3u) {
        error = "Audio buffer is empty or exceeds the RIFF/WAV 24-bit size limit.";
        return false;
    }
    std::ofstream f(path, std::ios::binary);
    if (!f) { error = "Cannot create WAV: " + path.string(); return false; }
    const auto bytes = static_cast<std::uint32_t>(a.samples.size() * 3u);
    f.write("RIFF", 4); put32(f, 36u + bytes); f.write("WAVEfmt ", 8);
    put32(f, 16u); put16(f, 1u); put16(f, 2u);
    put32(f, static_cast<std::uint32_t>(a.sampleRate));
    put32(f, static_cast<std::uint32_t>(a.sampleRate * 6));
    put16(f, 6u); put16(f, 24u); f.write("data", 4); put32(f, bytes);
    for (float sample : a.samples) {
        const double scaled = std::clamp(static_cast<double>(sample), -1.0, 1.0) * 8388607.0;
        const auto value = static_cast<std::int32_t>(std::lrint(scaled));
        const auto raw = static_cast<std::uint32_t>(value);
        f.put(static_cast<char>(raw & 0xffu));
        f.put(static_cast<char>((raw >> 8u) & 0xffu));
        f.put(static_cast<char>((raw >> 16u) & 0xffu));
    }
    if (!f) { error = "WAV write failed: " + path.string(); return false; }
    return true;
}
std::string keyName(int root) {
    static const std::array<const char*, 12> keys{"C","Csharp","D","Dsharp","E","F","Fsharp","G","Gsharp","A","Asharp","B"};
    return keys[static_cast<std::size_t>(std::clamp(root, 0, 11))];
}
} // namespace

std::string validate(const Settings& s) {
    if (s.bpm < 40 || s.bpm > 200) return "BPM must be between 40 and 200.";
    if (s.rootNote < 0 || s.rootNote > 11) return "Root note must be between C and B.";
    if (s.sampleRate != 48000 && s.sampleRate != 96000) return "CINEFORGE exports at 48 or 96 kHz.";
    if (s.packSize < 8 || s.packSize > 12) return "Pack size must be between 8 and 12 assets.";
    if (s.variations < 1 || s.variations > 12) return "Variation count must be between 1 and 12.";
    if (s.mood < 0 || s.mood > 100 || s.space < 0 || s.space > 100 || s.punch < 0 || s.punch > 100)
        return "Mood, Space and Punch must be between 0 and 100.";
    if (s.scene.size() > 512) return "Scene prompt must be 512 characters or fewer.";
    return {};
}

Result generate(const Settings& s, const std::filesystem::path& outputDirectory) {
    Result result;
    result.error = validate(s);
    if (!result.error.empty()) return result;
    std::error_code ec;
    std::filesystem::create_directories(outputDirectory, ec);
    if (ec) { result.error = "Cannot create output directory: " + ec.message(); return result; }
    const double root = midiHz(48 + s.rootNote);
    const double beat = 60.0 / s.bpm;
    const double seconds = std::clamp(beat * 2.0, 0.65, 3.0);
    AudioBuffer preview = makeBuffer(s.sampleRate, seconds);
    std::ostringstream manifest;
    manifest << "{\n  \"schema\": \"cineforge.pack.v1\",\n"
             << "  \"scene\": \"" ;
    for (char c : s.scene) { if (c == '"' || c == '\\') manifest << '\\'; if (static_cast<unsigned char>(c) >= 0x20) manifest << c; }
    manifest << "\",\n  \"bpm\": " << s.bpm << ",\n  \"key\": \"" << keyName(s.rootNote)
             << "\",\n  \"sample_rate\": " << s.sampleRate << ",\n  \"bit_depth\": 24,"
             << "\n  \"seed\": " << s.seed << ",\n  \"assets\": [\n";
    for (int i = 0; i < s.packSize; ++i) {
        const std::string name = names[static_cast<std::size_t>(i)];
        AudioBuffer dry = makeBuffer(s.sampleRate, seconds);
        renderAsset(dry, i, s, i % s.variations, root);
        normalize(dry);
        AudioBuffer wet = wetVersion(dry, s.space);
        normalize(wet);
        const std::string stem = "CF_" + std::to_string(s.bpm) + "bpm_" + keyName(s.rootNote) + "_" + name + "_";
        const auto dryPath = outputDirectory / (stem + "Dry.wav");
        const auto wetPath = outputDirectory / (stem + "Wet.wav");
        if (!write24(dryPath, dry, result.error) || !write24(wetPath, wet, result.error)) return result;
        result.assets.push_back({name, dryPath, wetPath, measure(dry)});
        if (i < 3) {
            for (std::size_t n = 0; n < preview.samples.size(); ++n)
                preview.samples[n] += dry.samples[n] * (i == 0 ? 0.62F : 0.34F);
        }
        manifest << "    {\"name\": \"" << name << "\", \"dry\": \"" << dryPath.filename().string()
                 << "\", \"wet\": \"" << wetPath.filename().string() << "\"}"
                 << (i + 1 == s.packSize ? "\n" : ",\n");
    }
    normalize(preview);
    result.previewPath = outputDirectory / ("CF_" + std::to_string(s.bpm) + "bpm_" + keyName(s.rootNote) + "_Preview.wav");
    if (!write24(result.previewPath, preview, result.error)) return result;
    manifest << "  ],\n  \"preview\": \"" << result.previewPath.filename().string()
             << "\",\n  \"processing\": {\"mood\": " << s.mood << ", \"space\": " << s.space
             << ", \"punch\": " << s.punch << ", \"variations\": " << s.variations
             << ", \"headroom_dbfs\": -1.0, \"low_end_mono\": true}\n}\n";
    result.manifestPath = outputDirectory / "cineforge_manifest.json";
    std::ofstream json(result.manifestPath, std::ios::binary);
    json << manifest.str();
    if (!json) { result.error = "Cannot write CINEFORGE manifest."; return result; }
    return result;
}

} // namespace ventu::cineforge
