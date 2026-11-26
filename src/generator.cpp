#include "generator.hpp"
#include "stochastic_router.hpp"
#include "audio_telemetry.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <cctype>
#include <limits>
#include <random>
#include <sstream>
#include <system_error>

namespace ventu {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kCeiling = 0.96605087899; // -0.3 dBFS sample-peak ceiling
constexpr std::uint64_t kMaxSourceBytes = 128ull * 1024ull * 1024ull;
constexpr std::size_t kMaxSourceFrames = 20u * 1000u * 1000u;

struct StyleProfile {
    std::array<int, 6> kicks;
    int kickCount;
    int snareA;
    int snareB;
    bool halfTime;
    double hatProbability;
    double toneDecay;
    double bassWeight;
    double kickWeight{1.0};
    double snareWeight{1.0};
    double hatWeight{1.0};
};

StyleProfile profile(Style style) {
    switch (style) {
    case Style::BoomBap:       return {{{0, 6, 8, 14, 0, 0}}, 4, 4, 12, false, 0.48, 0.85, 0.72};
    case Style::EastCoast:     return {{{0, 5, 8, 11, 14, 0}}, 5, 4, 12, false, 0.52, 0.78, 0.76};
    case Style::HarlemDipTrap: return {{{0, 6, 10, 14, 0, 0}}, 4, 4, 12, false, 0.58, 0.92, 0.82};
    case Style::GUnit:         return {{{0, 7, 8, 14, 0, 0}}, 4, 4, 12, false, 0.43, 0.72, 0.90};
    case Style::ModernTrap:    return {{{0, 7, 10, 14, 0, 0}}, 4, 8, -1, true, 0.72, 0.52, 1.00};
    case Style::AtlantaTrap:   return {{{0, 6, 10, 12, 14, 0}}, 5, 8, -1, true, 0.78, 0.48, 1.00};
    case Style::UKDrill:        return {{{0, 3, 7, 10, 14, 0}}, 5, 8, -1, true, 0.66, 0.56, 1.00};
    case Style::UKGrime:        return {{{0, 4, 7, 11, 14, 0}}, 5, 4, 12, false, 0.68, 0.55, 0.90};
    case Style::Memphis:       return {{{0, 5, 8, 13, 0, 0}}, 4, 8, -1, true, 0.74, 0.44, 1.00};
    case Style::JazzRap:       return {{{0, 7, 10, 14, 0, 0}}, 4, 4, 12, false, 0.45, 1.15, 0.65};
    case Style::Gfunk:         return {{{0, 6, 8, 12, 14, 0}}, 5, 4, 12, false, 0.50, 1.00, 0.82};
    case Style::BoomTrap:      return {{{0, 6, 8, 10, 14, 0}}, 5, 4, 12, false, 0.65, 0.74, 0.96};
    case Style::CrippleTrill:  return {{{0, 3, 8, 11, 14, 0}}, 5, 4, 12, false, 0.70, 0.62, 0.95};
    case Style::BillionaireGoldenEra:
        return {{{0, 6, 8, 14, 0, 0}}, 4, 4, 12, false, 0.34, 0.90, 0.95,
                1.10, 1.12, 0.78};
    }
    return {{{0, 5, 8, 14, 0, 0}}, 4, 4, 12, false, 0.52, 0.8, 0.8};
}

std::vector<int> scaleIntervals(Scale scale) {
    switch (scale) {
    case Scale::NaturalMinor: return {0, 2, 3, 5, 7, 8, 10};
    case Scale::HarmonicMinor: return {0, 2, 3, 5, 7, 8, 11};
    case Scale::Dorian: return {0, 2, 3, 5, 7, 9, 10};
    case Scale::Phrygian: return {0, 1, 3, 5, 7, 8, 10};
    case Scale::Major: return {0, 2, 4, 5, 7, 9, 11};
    case Scale::MinorPentatonic: return {0, 3, 5, 7, 10};
    }
    return {0, 2, 3, 5, 7, 8, 10};
}

double midiHz(int midi) { return 440.0 * std::pow(2.0, (static_cast<double>(midi) - 69.0) / 12.0); }
double uniform(std::mt19937& rng, double lo, double hi) {
    return std::uniform_real_distribution<double>(lo, hi)(rng);
}
int choose(std::mt19937& rng, int lo, int hi) {
    return std::uniform_int_distribution<int>(lo, hi)(rng);
}

void addStereo(AudioBuffer& audio, std::size_t frame, double value, double pan, double width) {
    if (frame >= audio.frames()) return;
    pan = std::clamp(pan * width, -1.0, 1.0);
    const double left = std::sqrt((1.0 - pan) * 0.5);
    const double right = std::sqrt((1.0 + pan) * 0.5);
    const auto base = frame * 2;
    audio.samples[base] += static_cast<float>(value * left);
    audio.samples[base + 1] += static_cast<float>(value * right);
}

void addKick(AudioBuffer& audio, std::size_t start, double amp, std::mt19937& rng, double width) {
    const int rate = audio.sampleRate;
    const auto length = static_cast<std::size_t>(rate * 0.48);
    double phase = 0.0;
    const double click = uniform(rng, 0.03, 0.12);
    for (std::size_t i = 0; i < length && start + i < audio.frames(); ++i) {
        const double t = static_cast<double>(i) / rate;
        const double frequency = 43.0 + 105.0 * std::exp(-t * 24.0);
        phase += 2.0 * kPi * frequency / rate;
        const double body = std::sin(phase) * std::exp(-t * 7.8);
        const double transient = (i < static_cast<std::size_t>(rate * 0.008))
            ? click * std::exp(-t * 520.0) * uniform(rng, -1.0, 1.0) : 0.0;
        addStereo(audio, start + i, std::tanh((body + transient) * 1.55) * amp, 0.0, width);
    }
}

void addSnare(AudioBuffer& audio, std::size_t start, double amp, std::mt19937& rng, double pan, double width) {
    const int rate = audio.sampleRate;
    const auto length = static_cast<std::size_t>(rate * 0.32);
    double previous = 0.0;
    double phase = 0.0;
    for (std::size_t i = 0; i < length && start + i < audio.frames(); ++i) {
        const double t = static_cast<double>(i) / rate;
        const double noise = uniform(rng, -1.0, 1.0);
        const double high = noise - previous * 0.86;
        previous = noise;
        phase += 2.0 * kPi * (178.0 + 14.0 * std::exp(-t * 22.0)) / rate;
        const double body = std::sin(phase) * std::exp(-t * 19.0) * 0.34;
        const double snap = high * std::exp(-t * 15.0) * 0.68;
        addStereo(audio, start + i, std::tanh((body + snap) * 1.15) * amp, pan, width);
    }
}

void addHat(AudioBuffer& audio, std::size_t start, double amp, std::mt19937& rng, double pan, double width, double decay = 0.055) {
    const int rate = audio.sampleRate;
    const auto length = static_cast<std::size_t>(rate * decay);
    double previous = 0.0;
    for (std::size_t i = 0; i < length && start + i < audio.frames(); ++i) {
        const double t = static_cast<double>(i) / rate;
        const double noise = uniform(rng, -1.0, 1.0);
        const double high = noise - previous * 0.94;
        previous = noise;
        const double metal = 0.12 * std::sin(2.0 * kPi * 6240.0 * t)
                           + 0.08 * std::sin(2.0 * kPi * 9130.0 * t);
        const double env = std::exp(-t * (decay < 0.03 ? 90.0 : 48.0));
        addStereo(audio, start + i, (high * 0.55 + metal) * env * amp, pan, width);
    }
}

void addPerc(AudioBuffer& audio, std::size_t start, double amp, std::mt19937& rng, double pan, double width) {
    const int rate = audio.sampleRate;
    const auto length = static_cast<std::size_t>(rate * 0.19);
    const double f = uniform(rng, 620.0, 1450.0);
    for (std::size_t i = 0; i < length && start + i < audio.frames(); ++i) {
        const double t = static_cast<double>(i) / rate;
        const double tone = std::sin(2.0 * kPi * f * t) * std::exp(-t * 27.0);
        const double tick = (i < static_cast<std::size_t>(rate * 0.003)) ? uniform(rng, -0.3, 0.3) : 0.0;
        addStereo(audio, start + i, (tone + tick) * amp, pan, width);
    }
}

void addTone(AudioBuffer& audio, std::size_t start, double frequency, double seconds,
             double amp, Timbre timbre, double pan, double width, double grit, std::mt19937& rng) {
    const int rate = audio.sampleRate;
    const auto length = static_cast<std::size_t>(std::max(0.02, seconds) * rate);
    const double attack = 0.008;
    for (std::size_t i = 0; i < length && start + i < audio.frames(); ++i) {
        const double t = static_cast<double>(i) / rate;
        const double env = std::min(1.0, t / attack) * std::exp(-t / std::max(0.04, seconds * 0.62));
        const double phase = 2.0 * kPi * frequency * t;
        double wave = std::sin(phase);
        switch (timbre) {
        case Timbre::Keys:    wave = 0.72 * std::sin(phase) + 0.20 * std::sin(phase * 2.01) + 0.08 * std::sin(phase * 3.98); break;
        case Timbre::Bell:    wave = 0.58 * std::sin(phase) + 0.28 * std::sin(phase * 2.76) + 0.14 * std::sin(phase * 5.41); break;
        case Timbre::Pluck:   wave = 0.72 * std::sin(phase) + 0.28 * std::sin(phase * 2.0); break;
        case Timbre::Organ:   wave = std::sin(phase) + 0.28 * std::sin(phase * 2.0) + 0.12 * std::sin(phase * 3.0); break;
        case Timbre::Brass:   wave = std::sin(phase) + 0.24 * std::sin(phase * 2.0) + 0.15 * std::sin(phase * 3.0); wave = std::tanh(wave * 0.72); break;
        case Timbre::Strings: wave = 0.7 * std::sin(phase) + 0.2 * std::sin(phase * 2.0) + 0.1 * std::sin(phase * 4.0); break;
        }
        const double noise = grit > 0.0 ? uniform(rng, -1.0, 1.0) * grit * 0.035 : 0.0;
        addStereo(audio, start + i, (wave + noise) * env * amp, pan, width);
    }
}

void addBass(AudioBuffer& audio, std::size_t start, double frequency, double seconds,
             double amp, double pan, double width, double glideSemitones = 0.0) {
    const int rate = audio.sampleRate;
    const auto length = static_cast<std::size_t>(std::max(0.035, seconds) * rate);
    double phase = 0.0;
    for (std::size_t i = 0; i < length && start + i < audio.frames(); ++i) {
        const double t = static_cast<double>(i) / rate;
        const double slide = glideSemitones * (1.0 - std::exp(-t * 12.0));
        const double hz = frequency * std::pow(2.0, slide / 12.0);
        phase += 2.0 * kPi * hz / rate;
        const double fundamental = std::sin(phase);
        const double harmonic = 0.22 * std::sin(phase * 2.0) + 0.10 * std::sin(phase * 3.0);
        const double env = std::min(1.0, t / 0.006) * std::exp(-t * 3.3);
        addStereo(audio, start + i, std::tanh((fundamental + harmonic) * 1.25) * env * amp, pan, width);
    }
}

void mixSourceChops(AudioBuffer& output, const AudioBuffer& source, const Parameters& p,
                    int variation) {
    if (source.samples.empty() || source.frames() < 4u || (source.channels != 1 && source.channels != 2)) return;
    const std::size_t sliceFrames = source.frames() / 4u;
    if (sliceFrames == 0u) return;
    const double pitchRatio = std::pow(2.0, static_cast<double>(p.sourcePitchSemitones) / 12.0);
    const double rateRatio = (static_cast<double>(source.sampleRate) / output.sampleRate) * pitchRatio;
    const double gain = static_cast<double>(p.sourceMixPercent) / 100.0 * 0.72;
    const double beatFrames = (60.0 / p.bpm) * output.sampleRate;
    const double width = static_cast<double>(p.stereoPercent) / 100.0;
    for (int bar = 0; bar < p.bars; ++bar) {
        for (int beat = 0; beat < 4; ++beat) {
            const int rotation = (variation * 3 + bar * 2) % 4;
            constexpr std::array<std::size_t, 4> crateOrder{0u, 2u, 1u, 3u};
            const std::size_t sliceIndex = p.style == Style::BillionaireGoldenEra
                ? crateOrder[static_cast<std::size_t>(beat)]
                : static_cast<std::size_t>((beat + rotation) % 4);
            const std::size_t sourceStart = sliceIndex * sliceFrames;
            const auto outputStart = static_cast<std::size_t>(
                (static_cast<double>(bar * 4 + beat) * beatFrames));
            const auto maxOutputFrames = static_cast<std::size_t>(std::max(1.0, beatFrames));
            const auto availableOutputFrames = static_cast<std::size_t>(
                std::ceil(static_cast<double>(sliceFrames) / std::max(0.01, rateRatio)));
            const std::size_t frames = std::min(maxOutputFrames, availableOutputFrames);
            const double pan = (beat % 2 == 0 ? -0.12 : 0.12) * width;
            for (std::size_t frame = 0; frame < frames && outputStart + frame < output.frames(); ++frame) {
                const double sourcePosition = static_cast<double>(frame) * rateRatio;
                const auto sourceIndex = static_cast<std::size_t>(sourcePosition);
                if (sourceIndex >= sliceFrames) break;
                const auto nextIndex = std::min(sourceIndex + 1u, sliceFrames - 1u);
                const double fraction = sourcePosition - static_cast<double>(sourceIndex);
                const auto baseA = (sourceStart + sourceIndex) * static_cast<std::size_t>(source.channels);
                const auto baseB = (sourceStart + nextIndex) * static_cast<std::size_t>(source.channels);
                const double leftA = source.samples[baseA];
                const double leftB = source.samples[baseB];
                const double rightA = source.channels == 2 ? source.samples[baseA + 1u] : leftA;
                const double rightB = source.channels == 2 ? source.samples[baseB + 1u] : leftB;
                double left = leftA + (leftB - leftA) * fraction;
                double right = rightA + (rightB - rightA) * fraction;
                const double attack = std::min(1.0, static_cast<double>(frame) / (output.sampleRate * 0.004));
                const double remaining = static_cast<double>(frames - frame) / (output.sampleRate * 0.018);
                const double release = std::min(1.0, remaining);
                const double envelope = std::min(attack, release) * gain;
                const double mid = (left + right) * 0.5;
                const double side = (left - right) * 0.5 * width;
                left = mid + side;
                right = mid - side;
                const double panL = std::sqrt((1.0 - pan) * 0.5);
                const double panR = std::sqrt((1.0 + pan) * 0.5);
                const auto destination = (outputStart + frame) * 2u;
                output.samples[destination] += static_cast<float>(left * envelope * panL * 1.41421356237);
                output.samples[destination + 1u] += static_cast<float>(right * envelope * panR * 1.41421356237);
            }
        }
    }
}

std::filesystem::path pathFromUtf8(const std::string& value) {
    const auto* first = reinterpret_cast<const char8_t*>(value.data());
    return std::filesystem::path(std::u8string(first, first + value.size()));
}

std::uint16_t readU16(const unsigned char* p) {
    return static_cast<std::uint16_t>(p[0] | (static_cast<std::uint16_t>(p[1]) << 8u));
}
std::uint32_t readU32(const unsigned char* p) {
    return static_cast<std::uint32_t>(p[0]) |
           (static_cast<std::uint32_t>(p[1]) << 8u) |
           (static_cast<std::uint32_t>(p[2]) << 16u) |
           (static_cast<std::uint32_t>(p[3]) << 24u);
}

void applySamplerColor(AudioBuffer& audio, Sampler sampler, int gritPercent) {
    int bits = 16;
    int hold = 1;
    double drive = 1.0 + static_cast<double>(gritPercent) / 140.0;
    if (sampler == Sampler::MPC2500) { bits = 12; hold = 2; drive += 0.18; }
    if (sampler == Sampler::MPC3000XL) { bits = 14; hold = 1; drive += 0.10; }
    if (sampler == Sampler::MPC5000) { bits = 16; hold = 1; drive += 0.04; }
    const double levels = std::pow(2.0, bits - 1);
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        if (hold > 1 && frame % static_cast<std::size_t>(hold) == 1 && frame > 0) {
            audio.samples[frame * 2] = audio.samples[(frame - 1) * 2];
            audio.samples[frame * 2 + 1] = audio.samples[(frame - 1) * 2 + 1];
        }
        for (int channel = 0; channel < 2; ++channel) {
            auto& x = audio.samples[frame * 2 + static_cast<std::size_t>(channel)];
            const double shaped = std::tanh(static_cast<double>(x) * drive) / std::tanh(drive);
            x = static_cast<float>(std::round(shaped * levels) / levels);
        }
    }
}

void applyMaster(AudioBuffer& audio, const Parameters& p) {
    const double hpAlpha = std::exp(-2.0 * kPi * p.highPassHz / audio.sampleRate);
    const double lpAlpha = 1.0 - std::exp(-2.0 * kPi * p.lowPassHz / audio.sampleRate);
    std::array<double, 2> previousInput{0.0, 0.0};
    std::array<double, 2> previousLow{0.0, 0.0};
    double envelope = 0.0;
    const double threshold = 0.72;
    const double ratio = std::clamp(p.busCompressionRatio, 1.0, 12.0);
    const double attack = 1.0 - std::exp(-1.0 / (0.004 * audio.sampleRate));
    const double release = 1.0 - std::exp(-1.0 / (0.085 * audio.sampleRate));
    const double saturation = 1.0 + static_cast<double>(p.saturationPercent) / 45.0;

    // A single detector and gain link both channels. This keeps stereo balance
    // stable when one side contains a stronger transient than the other.
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        std::array<double, 2> filtered{};
        double detector = 0.0;
        for (int channel = 0; channel < 2; ++channel) {
            const auto index = static_cast<std::size_t>(channel);
            const double input = audio.samples[frame * 2 + index];
            const double high = input - hpAlpha * previousInput[index];
            previousInput[index] = input;
            previousLow[index] += lpAlpha * (high - previousLow[index]);
            filtered[index] = previousLow[index];
            detector = std::max(detector, std::abs(filtered[index]));
        }

        const double coefficient = detector > envelope ? attack : release;
        envelope += coefficient * (detector - envelope);
        double gain = 1.0;
        if (envelope > threshold) {
            gain = std::pow(threshold / envelope, 1.0 - 1.0 / ratio);
        }

        for (int channel = 0; channel < 2; ++channel) {
            const auto index = static_cast<std::size_t>(channel);
            const double compressed = filtered[index] * gain;
            const double shaped = std::tanh(compressed * saturation) / std::tanh(saturation);
            audio.samples[frame * 2 + index] = static_cast<float>(shaped);
        }
    }

    // This is a sample-peak ceiling, not an oversampled true-peak limiter.
    const auto peak = measure(audio).peak;
    if (peak > kCeiling) {
        const double gain = kCeiling / peak;
        for (auto& sample : audio.samples) sample = static_cast<float>(sample * gain);
    }
}

std::string jsonEscape(const std::string& input) {
    std::ostringstream out;
    for (unsigned char c : input) {
        switch (c) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default: if (c < 0x20) out << ' '; else out << static_cast<char>(c);
        }
    }
    return out.str();
}

std::string slug(const std::string& value) {
    std::string result;
    for (unsigned char c : value) {
        if (std::isalnum(c)) result.push_back(static_cast<char>(std::tolower(c)));
        else if (c == ' ' || c == '-' || c == '_') result.push_back('_');
    }
    while (!result.empty() && result.back() == '_') result.pop_back();
    return result;
}

std::string modeLabel(Scale scale) {
    switch (scale) {
    case Scale::NaturalMinor: return "Minor";
    case Scale::HarmonicMinor: return "Harmonic Minor";
    case Scale::Dorian: return "Dorian";
    case Scale::Phrygian: return "Phrygian";
    case Scale::Major: return "Major";
    case Scale::MinorPentatonic: return "Minor Pentatonic";
    }
    return "Minor";
}

std::string modeFamily(Scale scale) {
    switch (scale) {
    case Scale::Major: return "major";
    case Scale::NaturalMinor:
    case Scale::HarmonicMinor:
    case Scale::MinorPentatonic: return "minor";
    case Scale::Dorian:
    case Scale::Phrygian: return "modal";
    }
    return "minor";
}

std::string musicalKeyLabel(int rootNote, Scale scale) {
    static constexpr std::array<const char*, 12> names{
        "C", "Csharp", "D", "Dsharp", "E", "F", "Fsharp", "G", "Gsharp", "A", "Asharp", "B"
    };
    const auto index = static_cast<std::size_t>(std::clamp(rootNote, 0, 11));
    return std::string(names[index]) + " " + modeLabel(scale);
}

std::string musicalKeyFileTag(int rootNote, Scale scale) {
    auto label = musicalKeyLabel(rootNote, scale);
    std::replace(label.begin(), label.end(), ' ', '_');
    return label;
}

void addOneShot(AudioBuffer& audio, const Parameters& p, const std::string& name, std::mt19937& rng) {
    if (name == "kick") addKick(audio, 0, 0.88, rng, 0.0);
    else if (name == "snare") addSnare(audio, 0, 0.76, rng, 0.0, 0.0);
    else if (name == "hat") addHat(audio, 0, 0.62, rng, 0.0, 0.0, 0.07);
    else if (name == "clap") {
        addSnare(audio, 0, 0.54, rng, 0.0, 0.0);
        addSnare(audio, static_cast<std::size_t>(audio.sampleRate * 0.012), 0.35, rng, 0.0, 0.0);
    } else if (name == "rim") addPerc(audio, 0, 0.82, rng, 0.0, 0.0);
    else if (name == "perc") {
        addPerc(audio, 0, 0.68, rng, 0.0, 0.0);
        addHat(audio, static_cast<std::size_t>(audio.sampleRate * 0.025), 0.22, rng, 0.0, 0.0, 0.025);
    } else if (name == "bass") addBass(audio, 0, midiHz(36 + p.rootNote), 0.72, 0.94, 0.0, 0.0, -2.0);
    else if (name == "pluck") addTone(audio, 0, midiHz(60 + p.rootNote), 0.82, 0.78, Timbre::Pluck, 0.0, 0.0, p.gritPercent / 100.0, rng);
    else if (name == "fx") {
        addTone(audio, 0, midiHz(72 + p.rootNote), 0.7, 0.5, Timbre::Bell, 0.0, 0.0, 0.1, rng);
        addHat(audio, static_cast<std::size_t>(audio.sampleRate * 0.09), 0.22, rng, 0.0, 0.0, 0.23);
    }
}

} // namespace

std::string toString(Style value) {
    switch (value) {
    case Style::BoomBap: return "Boom Bap";
    case Style::EastCoast: return "East Coast";
    case Style::HarlemDipTrap: return "Harlem Dip-Trap";
    case Style::GUnit: return "G-Unit";
    case Style::ModernTrap: return "Modern Trap";
    case Style::AtlantaTrap: return "Atlanta Trap";
    case Style::UKDrill: return "UK Drill";
    case Style::UKGrime: return "UK Grime";
    case Style::Memphis: return "Memphis";
    case Style::JazzRap: return "Jazz Rap";
    case Style::Gfunk: return "G-Funk";
    case Style::BoomTrap: return "Boom Trap";
    case Style::CrippleTrill: return "Cripple Trill";
    case Style::BillionaireGoldenEra: return "Billionaire Golden Era";
    }
    return "East Coast";
}
std::string toString(Target value) {
    switch (value) {
    case Target::FullLoop: return "Full Loop";
    case Target::Melody: return "Melody";
    case Target::Drums: return "Drums";
    case Target::Bass: return "Bass";
    case Target::OneShotKit: return "One-Shot Kit";
    }
    return "Full Loop";
}
std::string toString(Scale value) {
    switch (value) {
    case Scale::NaturalMinor: return "Natural Minor";
    case Scale::HarmonicMinor: return "Harmonic Minor";
    case Scale::Dorian: return "Dorian";
    case Scale::Phrygian: return "Phrygian";
    case Scale::Major: return "Major";
    case Scale::MinorPentatonic: return "Minor Pentatonic";
    }
    return "Natural Minor";
}
std::string toString(Sampler value) {
    switch (value) {
    case Sampler::Clean: return "Clean";
    case Sampler::MPC2500: return "MPC 2500";
    case Sampler::MPC3000XL: return "MPC 3000XL";
    case Sampler::MPC5000: return "MPC 5000";
    }
    return "MPC 3000XL";
}
std::string toString(Timbre value) {
    switch (value) {
    case Timbre::Keys: return "Keys";
    case Timbre::Bell: return "Bell";
    case Timbre::Pluck: return "Pluck";
    case Timbre::Organ: return "Organ";
    case Timbre::Brass: return "Brass";
    case Timbre::Strings: return "Strings";
    }
    return "Keys";
}

std::string validate(const Parameters& p) {
    if (p.bpm < 60 || p.bpm > 190) return "BPM должен находиться в диапазоне 60–190.";
    if (p.rootNote < 0 || p.rootNote > 11) return "Тональность должна быть в диапазоне 0–11.";
    if (p.bars < 1 || p.bars > 16) return "Длина должна составлять 1–16 тактов.";
    if (p.variations < 1 || p.variations > 32) return "Количество вариантов должно быть в диапазоне 1–32.";
    if (p.swingPercent < 50 || p.swingPercent > 68) return "Swing должен находиться в диапазоне 50–68%.";
    if (p.stochasticIntensityPercent < 0 || p.stochasticIntensityPercent > 100)
        return "Интенсивность стохастической цепи должна находиться в диапазоне 0–100%.";
    if (p.stochasticTriggerMode < 0 || p.stochasticTriggerMode > 2)
        return "Неизвестный режим триггера стохастической маршрутизации.";
    if (p.stochasticTopology < 0 || p.stochasticTopology > 2)
        return "Неизвестный тип графа стохастической маршрутизации.";
    if ((p.stochasticPluginMask & ~0xFFu) != 0u)
        return "Маска пула стохастических эффектов содержит неизвестные биты.";
    if (p.stochasticEnabled && p.stochasticIntensityPercent > 0 && p.stochasticPluginMask == 0u)
        return "Включи хотя бы один DSP-эффект в пуле стохастической маршрутизации.";
    if (p.densityPercent < 0 || p.densityPercent > 100 || p.gritPercent < 0 || p.gritPercent > 100 ||
        p.saturationPercent < 0 || p.saturationPercent > 100 || p.stereoPercent < 0 || p.stereoPercent > 100)
        return "Проценты плотности, зерна, сатурации и ширины должны находиться в диапазоне 0–100.";
    if (p.highPassHz < 20 || p.highPassHz > 300 || p.lowPassHz < 2000 || p.lowPassHz > 20000 || p.lowPassHz <= p.highPassHz || p.lowPassHz > static_cast<int>(p.sampleRate * 0.49))
        return "Проверьте фильтры: HP 20–300 Гц, LP 2–20 кГц и ниже частоты Найквиста.";
    if (p.snareOffsetMs < -30 || p.snareOffsetMs > 30) return "Смещение малого барабана должно быть в диапазоне ±30 мс.";
    if (p.sampleRate != 22050 && p.sampleRate != 44100 && p.sampleRate != 48000 && p.sampleRate != 96000)
        return "Поддерживаются частоты дискретизации 22050, 44100, 48000 и 96000 Гц.";
    if (p.sourceMixPercent < 0 || p.sourceMixPercent > 100 || p.sourcePitchSemitones < -12 || p.sourcePitchSemitones > 12)
        return "Уровень исходного сэмпла должен быть 0–100%, а транспонирование — ±12 полутонов.";
    if (!std::isfinite(p.busCompressionRatio) || p.busCompressionRatio < 1.0 || p.busCompressionRatio > 12.0)
        return "Коэффициент компрессии должен находиться в диапазоне 1:1–12:1.";
    if (!std::isfinite(p.targetPeakDbfs) || p.targetPeakDbfs < -24.0 || p.targetPeakDbfs > -0.3)
        return "Целевой пик должен находиться в диапазоне -24.0…-0.3 dBFS.";
    const double durationSeconds = p.target == Target::OneShotKit ? 1.35 : (60.0 / p.bpm) * 4.0 * p.bars;
    const double outputCount = static_cast<double>(p.variations) * (p.target == Target::OneShotKit ? 8.0 : 1.0);
    const double estimatedBytes = durationSeconds * p.sampleRate * 4.0 * outputCount;
    if (estimatedBytes > 512.0 * 1024.0 * 1024.0)
        return "Расчётный объём sample pack превышает лимит 512 МиБ. Уменьши длину, частоту дискретизации или число вариаций.";
    return {};
}

AudioBuffer loadWave(const std::filesystem::path& path, std::string& error) {
    error.clear();
    AudioBuffer audio;
    std::error_code fileError;
    const auto fileBytes = std::filesystem::file_size(path, fileError);
    if (fileError || fileBytes < 44u || fileBytes > kMaxSourceBytes) {
        error = "Исходный WAV отсутствует, слишком мал или превышает лимит 128 МиБ.";
        return {};
    }
    std::ifstream file(path, std::ios::binary);
    if (!file) { error = "Не удалось открыть исходный WAV: " + path.string(); return {}; }
    std::array<unsigned char, 12> riff{};
    file.read(reinterpret_cast<char*>(riff.data()), static_cast<std::streamsize>(riff.size()));
    if (!file || std::string(reinterpret_cast<const char*>(riff.data()), 4) != "RIFF" ||
        std::string(reinterpret_cast<const char*>(riff.data() + 8), 4) != "WAVE") {
        error = "Исходный файл не является RIFF/WAVE.";
        return {};
    }
    const auto riffPayloadBytes = static_cast<std::uint64_t>(readU32(riff.data() + 4));
    if (riffPayloadBytes < 36u || riffPayloadBytes > fileBytes - 8u) {
        error = "Некорректный RIFF-размер исходного WAV.";
        return {};
    }
    unsigned format = 0, channels = 0, sampleRate = 0, bits = 0, blockAlign = 0;
    std::vector<unsigned char> pcm;
    while (file && static_cast<std::uint64_t>(file.tellg()) + 8u <= fileBytes) {
        std::array<unsigned char, 8> chunk{};
        file.read(reinterpret_cast<char*>(chunk.data()), static_cast<std::streamsize>(chunk.size()));
        if (!file) break;
        const std::uint32_t chunkSize = readU32(chunk.data() + 4);
        const auto payloadPosition = static_cast<std::uint64_t>(file.tellg());
        if (chunkSize > fileBytes || payloadPosition + chunkSize > fileBytes) {
            error = "Повреждённая структура WAV: размер блока выходит за границы файла.";
            return {};
        }
        const std::string chunkId(reinterpret_cast<const char*>(chunk.data()), 4);
        if (chunkId == "fmt ") {
            if (chunkSize < 16u || chunkSize > 4096u) {
                error = "Некорректный или неподдерживаемый fmt-блок WAV.";
                return {};
            }
            std::vector<unsigned char> fmt(chunkSize);
            file.read(reinterpret_cast<char*>(fmt.data()), static_cast<std::streamsize>(fmt.size()));
            if (!file) { error = "Не удалось прочитать fmt-блок WAV."; return {}; }
            format = readU16(fmt.data());
            channels = readU16(fmt.data() + 2);
            sampleRate = readU32(fmt.data() + 4);
            blockAlign = readU16(fmt.data() + 12);
            bits = readU16(fmt.data() + 14);
        } else if (chunkId == "data") {
            if (!pcm.empty() || chunkSize == 0u) {
                error = "WAV должен содержать один непустой data-блок.";
                return {};
            }
            pcm.resize(chunkSize);
            file.read(reinterpret_cast<char*>(pcm.data()), static_cast<std::streamsize>(pcm.size()));
            if (!file) { error = "Не удалось прочитать PCM-данные исходного WAV."; return {}; }
        }
        const auto next = static_cast<std::uint64_t>(file.tellg()) + (chunkSize & 1u);
        if (next > fileBytes) break;
        file.seekg(static_cast<std::streamoff>(next), std::ios::beg);
    }
    if (format != 1u || bits != 16u || (channels != 1u && channels != 2u) ||
        sampleRate < 8000u || sampleRate > 192000u || blockAlign != channels * 2u ||
        pcm.empty() || pcm.size() % blockAlign != 0u) {
        error = "Поддерживается только корректный PCM16 WAV: mono/stereo, 8–192 кГц.";
        return {};
    }
    const std::size_t frames = pcm.size() / blockAlign;
    if (frames < 4u || frames > kMaxSourceFrames) {
        error = "Длительность исходного WAV должна составлять от 4 кадров до 20 млн кадров.";
        return {};
    }
    audio.sampleRate = static_cast<int>(sampleRate);
    audio.channels = static_cast<int>(channels);
    audio.samples.resize(frames * channels);
    for (std::size_t i = 0; i < audio.samples.size(); ++i) {
        const auto raw = readU16(pcm.data() + i * 2u);
        const std::int32_t signedSample = raw >= 0x8000u ? static_cast<std::int32_t>(raw) - 0x10000 : static_cast<std::int32_t>(raw);
        audio.samples[i] = static_cast<float>(static_cast<double>(signedSample) / 32768.0);
    }
    return audio;
}

AudioBuffer render(const Parameters& p, int variation, const std::string& oneShot, const AudioBuffer* sourceSample) {
    AudioBuffer audio;
    if (!validate(p).empty()) return audio;
    audio.sampleRate = p.sampleRate;
    const bool kitHit = !oneShot.empty();
    const double duration = kitHit ? 1.35 : (60.0 / p.bpm) * 4.0 * p.bars;
    const auto frameCount = static_cast<std::size_t>(std::max(1.0, std::ceil(duration * p.sampleRate)));
    audio.samples.assign(frameCount * 2, 0.0F);
    const auto seed = static_cast<std::uint32_t>(p.seed) ^ (static_cast<std::uint32_t>(variation + 1) * 0x9E3779B9u);
    std::mt19937 rng(seed);
    const double width = static_cast<double>(p.stereoPercent) / 100.0;
    const double density = static_cast<double>(p.densityPercent) / 100.0;
    const double grit = static_cast<double>(p.gritPercent) / 100.0;
    const double stepFrames = (60.0 / p.bpm) * p.sampleRate / 4.0;

    if (kitHit) {
        addOneShot(audio, p, oneShot, rng);
    } else {
        const auto spec = profile(p.style);
        const auto intervals = scaleIntervals(p.scale);
        const int rootMidi = 48 + p.rootNote;
        const bool drumsOn = p.target == Target::FullLoop || p.target == Target::Drums;
        const bool melodyOn = p.target == Target::FullLoop || p.target == Target::Melody;
        const bool bassOn = p.target == Target::FullLoop || p.target == Target::Bass;
        const double barFrames = stepFrames * 16.0;
        const auto swingOffsetForStep = [&p, stepFrames](int step) {
            return (step % 4) == 2
                ? stepFrames * 4.0 * static_cast<double>(p.swingPercent - 50) / 100.0
                : 0.0;
        };

        for (int bar = 0; bar < p.bars; ++bar) {
            const bool phraseTurnaround = ((bar + 1) % 4) == 0;
            for (int step = 0; step < 16; ++step) {
                double onset = bar * barFrames + step * stepFrames + swingOffsetForStep(step);
                if (p.microHumanize) onset += uniform(rng, -0.002, 0.002) * p.sampleRate;
                if (onset < 0.0) onset = 0.0;
                const auto at = static_cast<std::size_t>(onset);

                const bool baseKick = std::find(spec.kicks.begin(), spec.kicks.begin() + spec.kickCount, step) != spec.kicks.begin() + spec.kickCount;
                const bool cadenceKick = phraseTurnaround && step == 15 && density >= 0.38 && p.style != Style::JazzRap;
                const bool kick = baseKick || cadenceKick;
                const bool baseSnare = step == spec.snareA || step == spec.snareB;
                const bool ghostSnare = drumsOn && phraseTurnaround && step == 10 && density >= 0.68
                    && uniform(rng, 0.0, 1.0) < 0.18;
                const bool snare = baseSnare || ghostSnare;

                if (drumsOn && kick) {
                    audio.eventFrames.push_back(at);
                    const double kickLevel = (cadenceKick && !baseKick)
                        ? 0.58 : 0.74 + uniform(rng, -0.08, 0.08);
                    addKick(audio, at, kickLevel * spec.kickWeight, rng, width);
                }
                if (drumsOn && snare) {
                    audio.eventFrames.push_back(at);
                    const auto offset = static_cast<long long>(p.snareOffsetMs) * p.sampleRate / 1000;
                    const auto shifted = static_cast<std::size_t>(std::max<long long>(0, static_cast<long long>(at) + offset));
                    const double snareLevel = ghostSnare
                        ? 0.28 : 0.64 + uniform(rng, -0.07, 0.07);
                    addSnare(audio, shifted, snareLevel * spec.snareWeight,
                             rng, uniform(rng, -0.08, 0.08), width);
                }

                const bool regularHat = drumsOn && (step % 2 == 0
                    || uniform(rng, 0.0, 1.0) < density * spec.hatProbability);
                const bool turnaroundHat = phraseTurnaround && step >= 13 && density >= 0.40;
                if (drumsOn && (regularHat || turnaroundHat)) {
                    audio.eventFrames.push_back(at);
                    const double accent = ((step % 4 == 0) ? 0.52
                        : (turnaroundHat ? 0.27 : 0.34)) * spec.hatWeight;
                    const double decay = turnaroundHat ? 0.026 : (step % 4 == 0 ? 0.065 : 0.042);
                    addHat(audio, at, accent, rng, uniform(rng, -0.78, 0.78), width, decay);
                }
                if (drumsOn && step % 4 == 3 && uniform(rng, 0.0, 1.0) < density * 0.25) {
                    audio.eventFrames.push_back(at);
                    addPerc(audio, at, 0.22, rng, uniform(rng, -0.85, 0.85), width);
                }
                if (drumsOn && p.tripletRolls && (p.style == Style::ModernTrap || p.style == Style::AtlantaTrap || p.style == Style::UKDrill || p.style == Style::Memphis || p.style == Style::CrippleTrill) && (step == 7 || step == 15)) {
                    for (int roll = 1; roll <= 2; ++roll) {
                        const auto rollAt = at + static_cast<std::size_t>(stepFrames * roll / 3.0);
                        if (rollAt < audio.frames()) addHat(audio, rollAt, 0.22, rng, uniform(rng, -0.8, 0.8), width, 0.023);
                    }
                }
            }

            const auto barStart = static_cast<std::size_t>(bar * barFrames);
            const int chordDegree = intervals[static_cast<std::size_t>((bar % 4 == 2) ? 3 : (bar % 4 == 3 ? 4 : 0))];
            if (melodyOn) {
                audio.eventFrames.push_back(barStart);
                const int third = intervals[std::min<std::size_t>(2, intervals.size() - 1)];
                const int fifth = intervals[std::min<std::size_t>(4, intervals.size() - 1)];
                const int chordNotes[] = {0, third, fifth};
                const double padLength = (60.0 / p.bpm) * 1.65;
                for (int i = 0; i < 3; ++i) {
                    const int degree = (chordNotes[i] + chordDegree) % 12;
                    const int midi = rootMidi + degree + (i == 2 ? 12 : 0);
                    addTone(audio, barStart, midiHz(midi), padLength, 0.12 + density * 0.04,
                            p.timbre, (i - 1) * 0.34, width, grit, rng);
                }
                const int notesPerBar = density < 0.3 ? 2 : (density < 0.7 ? 4 : 6);
                for (int n = 0; n < notesPerBar; ++n) {
                    const int step = (n * 16 / notesPerBar) + choose(rng, 0, 1);
                    const int degree = intervals[static_cast<std::size_t>(choose(rng, 0, static_cast<int>(intervals.size()) - 1))];
                    const int octave = choose(rng, 0, 1) * 12;
                    double noteOnset = bar * barFrames + step * stepFrames + swingOffsetForStep(step);
                    if (p.microHumanize) noteOnset += uniform(rng, -0.001, 0.001) * p.sampleRate;
                    const auto noteAt = static_cast<std::size_t>(std::max(0.0, noteOnset));
                    audio.eventFrames.push_back(noteAt);
                    addTone(audio, noteAt, midiHz(rootMidi + degree + octave), (60.0 / p.bpm) * uniform(rng, 0.12, 0.42),
                            uniform(rng, 0.12, 0.27), p.timbre, uniform(rng, -0.66, 0.66), width, grit, rng);
                }
            }
            if (bassOn) {
                for (int step : {0, 8}) {
                    const auto bassAt = static_cast<std::size_t>(bar * barFrames + step * stepFrames);
                    audio.eventFrames.push_back(bassAt);
                    const double slide = (p.style == Style::UKDrill || p.style == Style::ModernTrap || p.style == Style::AtlantaTrap) ? uniform(rng, -5.0, 7.0) : 0.0;
                    const int octave = (p.style == Style::JazzRap || p.style == Style::Gfunk) ? 1 : 0;
                    addBass(audio, bassAt, midiHz(rootMidi - 12 + octave * 12), (60.0 / p.bpm) * 1.6,
                            0.38 * spec.bassWeight, uniform(rng, -0.08, 0.08), width, slide);
                }
            }
        }
        if (sourceSample && p.sourceMixPercent > 0) mixSourceChops(audio, *sourceSample, p, variation);
        if (p.vinylDust && p.gritPercent > 0) {
            const auto specks = static_cast<int>(audio.frames() / static_cast<std::size_t>(p.sampleRate / 8));
            for (int i = 0; i < specks; ++i) {
                const auto at = static_cast<std::size_t>(choose(rng, 0, static_cast<int>(audio.frames() - 1)));
                addPerc(audio, at, grit * 0.025, rng, uniform(rng, -1.0, 1.0), width);
            }
        }
    }

    applySamplerColor(audio, p.sampler, p.gritPercent);
    applyMaster(audio, p);
    return audio;
}


CadenceStats analyzeCadence(const AudioBuffer& audio, int bpm, int swingPercent, double toleranceMs) {
    CadenceStats result;
    if (audio.sampleRate < 8000 || audio.sampleRate > 192000 || audio.channels != 2 ||
        audio.samples.empty() || audio.samples.size() % static_cast<std::size_t>(audio.channels) != 0 ||
        bpm < 60 || bpm > 190 || swingPercent < 50 || swingPercent > 68 ||
        !std::isfinite(toleranceMs) || toleranceMs <= 0.0 || toleranceMs > 50.0 ||
        audio.eventFrames.size() > 1000000u) {
        result.error = "Некорректные параметры аудио или сетки для анализа ритма.";
        return result;
    }
    result.valid = true;
    if (audio.eventFrames.empty()) return result;

    const double stepFrames = (60.0 / static_cast<double>(bpm)) *
                              static_cast<double>(audio.sampleRate) / 4.0;
    const double toleranceFrames = toleranceMs * static_cast<double>(audio.sampleRate) / 1000.0;
    double deviationSum = 0.0;
    result.eventCount = audio.eventFrames.size();
    for (const std::size_t frame : audio.eventFrames) {
        if (frame >= audio.frames()) {
            result.valid = false;
            result.error = "Событие выходит за границы аудиобуфера.";
            return result;
        }
        const auto nearestStep = static_cast<long long>(
            std::llround(static_cast<double>(frame) / stepFrames));
        double bestDeviation = std::numeric_limits<double>::max();
        for (long long candidate = nearestStep - 1; candidate <= nearestStep + 1; ++candidate) {
            if (candidate < 0) continue;
            const auto stepInBar = static_cast<int>(candidate % 16);
            double expectedFrame = static_cast<double>(candidate) * stepFrames;
            if ((stepInBar % 4) == 2) {
                expectedFrame += stepFrames * 4.0 *
                    static_cast<double>(swingPercent - 50) / 100.0;
            }
            bestDeviation = std::min(bestDeviation,
                std::abs(static_cast<double>(frame) - expectedFrame));
        }
        if (!std::isfinite(bestDeviation)) {
            result.valid = false;
            result.error = "Не удалось сопоставить событие с музыкальной сеткой.";
            return result;
        }
        deviationSum += bestDeviation;
        result.maxDeviationMs = std::max(
            result.maxDeviationMs,
            bestDeviation * 1000.0 / static_cast<double>(audio.sampleRate));
        if (bestDeviation <= toleranceFrames) ++result.onGridCount;
    }
    result.offGridCount = result.eventCount - result.onGridCount;
    result.meanDeviationMs = deviationSum * 1000.0 /
        (static_cast<double>(audio.sampleRate) * static_cast<double>(result.eventCount));
    result.gridAccuracyPercent = 100.0 * static_cast<double>(result.onGridCount) /
        static_cast<double>(result.eventCount);
    result.analyzed = true;
    return result;
}

AudioStats measure(const AudioBuffer& audio) {
    AudioStats stats;
    if (audio.samples.empty()) return stats;

    double sum = 0.0;
    double square = 0.0;
    double leftSquare = 0.0;
    double rightSquare = 0.0;
    double crossProduct = 0.0;
    for (std::size_t index = 0; index < audio.samples.size(); ++index) {
        const double value = std::isfinite(audio.samples[index])
            ? static_cast<double>(audio.samples[index]) : 0.0;
        stats.peak = std::max(stats.peak, std::abs(value));
        sum += value;
        square += value * value;

        if (audio.channels == 2) {
            if (index % 2u == 0u) {
                leftSquare += value * value;
            } else {
                rightSquare += value * value;
                const double left = std::isfinite(audio.samples[index - 1u])
                    ? static_cast<double>(audio.samples[index - 1u]) : 0.0;
                crossProduct += left * value;
            }
        }
    }

    stats.rms = std::sqrt(square / static_cast<double>(audio.samples.size()));
    stats.crestDb = stats.rms > 0.0 && stats.peak > 0.0
        ? 20.0 * std::log10(stats.peak / stats.rms) : 0.0;

    if (audio.channels == 2 && audio.samples.size() % 2u == 0u) {
        const double frameCount = static_cast<double>(audio.samples.size() / 2u);
        stats.leftRms = std::sqrt(leftSquare / frameCount);
        stats.rightRms = std::sqrt(rightSquare / frameCount);
        const double denominator = std::sqrt(leftSquare * rightSquare);
        stats.stereoCorrelation = denominator > 0.0
            ? std::clamp(crossProduct / denominator, -1.0, 1.0) : 0.0;
        stats.stereoCorrelationValid = true;
    }
    return stats;
}
bool writeWave(const std::filesystem::path& path, const AudioBuffer& audio,
               std::string& error, const WavExportOptions& options) {
    error.clear();
    if (audio.channels != 2 || audio.samples.size() % 2u != 0u ||
        audio.frames() == 0 || audio.sampleRate < 8000 || audio.sampleRate > 192000) {
        error = "Некорректная структура PCM-аудио.";
        return false;
    }
    const std::uint64_t dataBytes64 = static_cast<std::uint64_t>(audio.samples.size()) * 2u;
    if (dataBytes64 > std::numeric_limits<std::uint32_t>::max() - 36u) {
        error = "WAV превышает предел RIFF PCM16.";
        return false;
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        error = "Не удалось открыть файл для записи: " + path.string();
        return false;
    }
    const auto write16 = [&file](std::uint16_t value) {
        file.put(static_cast<char>(value & 0xffu));
        file.put(static_cast<char>((value >> 8u) & 0xffu));
    };
    const auto write32 = [&file](std::uint32_t value) {
        for (unsigned shift = 0; shift < 32; shift += 8)
            file.put(static_cast<char>((value >> shift) & 0xffu));
    };
    const auto nextRandom = [](std::uint32_t& state) {
        if (state == 0u) state = 0x6D2B79F5u;
        state ^= state << 13u;
        state ^= state >> 17u;
        state ^= state << 5u;
        return state;
    };

    std::uint32_t randomState = options.ditherSeed;
    const auto dataBytes = static_cast<std::uint32_t>(dataBytes64);
    file.write("RIFF", 4);
    write32(36u + dataBytes);
    file.write("WAVEfmt ", 8);
    write32(16u);
    write16(1u);
    write16(2u);
    write32(static_cast<std::uint32_t>(audio.sampleRate));
    write32(static_cast<std::uint32_t>(audio.sampleRate * 4));
    write16(4u);
    write16(16u);
    file.write("data", 4);
    write32(dataBytes);

    for (float sample : audio.samples) {
        const double clean = std::clamp(
            std::isfinite(sample) ? static_cast<double>(sample) : 0.0, -1.0, 1.0);
        double shaped = clean;
        if (options.tpdfDither) {
            const double first = static_cast<double>(nextRandom(randomState)) / 4294967295.0;
            const double second = static_cast<double>(nextRandom(randomState)) / 4294967295.0;
            shaped += (first - second) / 32767.0;
        }
        shaped = std::clamp(shaped, -1.0, 1.0);
        const auto pcm = static_cast<std::int16_t>(std::lrint(shaped * 32767.0));
        write16(static_cast<std::uint16_t>(pcm));
    }

    file.flush();
    if (!file.good()) {
        error = "Ошибка записи WAV: " + path.string();
        return false;
    }
    return true;
}

BatchResult generatePack(const Parameters& p, const std::filesystem::path& outputDirectory,
                          diagnostics::RenderTelemetry* telemetry) {
    BatchResult result;
    if (telemetry != nullptr) {
        const std::uint64_t filesPerVariation = p.target == Target::OneShotKit ? 8u : 1u;
        const auto variations = p.variations > 0 ? static_cast<std::uint64_t>(p.variations) : 0u;
        telemetry->beginJob(variations * filesPerVariation);
    }
    if (const auto issue = validate(p); !issue.empty()) { result.error = issue; return result; }
    std::error_code ec;
    std::filesystem::create_directories(outputDirectory, ec);
    if (ec) { result.error = "Не удалось создать папку экспорта: " + ec.message(); return result; }
    AudioBuffer sourceSample;
    const AudioBuffer* sourcePtr = nullptr;
    if (!p.sourceSamplePath.empty() && p.target != Target::OneShotKit) {
        std::string sourceError;
        sourceSample = loadWave(pathFromUtf8(p.sourceSamplePath), sourceError);
        if (sourceSample.samples.empty()) { result.error = sourceError; return result; }
        sourcePtr = &sourceSample;
    }
    const std::array<const char*, 8> kitNames{"kick", "snare", "hat", "clap", "rim", "perc", "bass", "pluck"};
    const std::array<const char*, 12> noteNames{"C", "Csharp", "D", "Dsharp", "E", "F", "Fsharp", "G", "Gsharp", "A", "Asharp", "B"};
    const std::string basePrefix = "VentuSample_" + slug(toString(p.style)) + "_" + std::to_string(p.bpm) + "BPM";
    const bool tonalTarget = p.target != Target::Drums && p.target != Target::OneShotKit;
    const std::string packPrefix = basePrefix + (tonalTarget ? "_" + musicalKeyFileTag(p.rootNote, p.scale) : "");
    const std::string selectedKeyLabel = musicalKeyLabel(p.rootNote, p.scale);
    const auto manifestPath = outputDirectory / "ventusample_manifest.json";
    if (std::filesystem::exists(manifestPath, ec) || ec) {
        result.error = "В выбранной папке уже есть ventusample_manifest.json или невозможно проверить содержимое. Выбери пустую папку экспорта.";
        return result;
    }
    // Preflight every destination before writing the first WAV. A collision in a
    // later variation must not leave a half-generated pack in the user's folder.
    for (int variation = 0; variation < p.variations; ++variation) {
        if (p.target == Target::OneShotKit) {
            for (const auto* hit : kitNames) {
                const bool pitchedHit = std::string(hit) == "bass" || std::string(hit) == "pluck";
                const std::string hitKeyTag = pitchedHit ? "_" + std::string(noteNames[static_cast<std::size_t>(p.rootNote)]) : "";
                const auto path = outputDirectory / (packPrefix + hitKeyTag + "_v" + std::to_string(variation + 1) + "_" + hit + ".wav");
                if (std::filesystem::exists(path, ec) || ec) {
                    result.error = "Файл уже существует или недоступен: " + path.string();
                    return result;
                }
            }
        } else {
            const auto path = outputDirectory / (packPrefix + "_" + slug(toString(p.target)) + "_v" + std::to_string(variation + 1) + ".wav");
            if (std::filesystem::exists(path, ec) || ec) {
                result.error = "Файл уже существует или недоступен: " + path.string();
                return result;
            }
        }
    }
    const auto applyStochastic = [&](AudioBuffer& audio, int variation, StochasticResult& report) -> bool {
        if (!p.stochasticEnabled || p.stochasticIntensityPercent <= 0) return true;
        StochasticConfig config;
        config.seed = static_cast<std::uint32_t>(p.seed)
            ^ (static_cast<std::uint32_t>(variation + 1) * 0x9E3779B9u);
        config.intensity = static_cast<double>(p.stochasticIntensityPercent) / 100.0;
        config.aggression = static_cast<double>(p.gritPercent) / 100.0;
        config.space = static_cast<double>(p.stereoPercent) / 100.0;
        config.dirt = static_cast<double>(p.saturationPercent) / 100.0;
        config.chaos = static_cast<double>(p.densityPercent) / 100.0;
        config.triggerMode = p.stochasticTriggerMode == 1
            ? StochasticTriggerMode::FixedBlocks
            : (p.stochasticTriggerMode == 2
                ? StochasticTriggerMode::ScheduledOnsets
                : StochasticTriggerMode::SinglePass);
        config.topology = p.stochasticTopology == 1
            ? StochasticTopology::Serial
            : (p.stochasticTopology == 2 ? StochasticTopology::Parallel : StochasticTopology::Hybrid);
        for (std::size_t effect = 0; effect < config.pool.size(); ++effect)
            config.pool[effect].enabled = (p.stochasticPluginMask & (1u << effect)) != 0u;
        config.blockFrames = std::max<std::size_t>(
            256u, static_cast<std::size_t>(audio.sampleRate) * 240u
                / static_cast<std::size_t>(p.bpm));
        report = processStochastic(audio, config);
        if (!report.success) {
            result.error = report.error;
            return false;
        }
        return true;
    };
    struct ExportMetadata {
        std::uint32_t ditherSeed{0};
        bool normalized{false};
        bool normalizationSkipped{false};
    };
    const auto prepareExport = [&](AudioBuffer& audio, int variation,
                                   const std::string& fileName) {
        ExportMetadata metadata;
        std::uint32_t seed = static_cast<std::uint32_t>(p.seed)
            ^ (static_cast<std::uint32_t>(variation + 1) * 0x9E3779B9u);
        for (unsigned char character : fileName)
            seed = (seed ^ static_cast<std::uint32_t>(character)) * 16777619u;
        metadata.ditherSeed = seed == 0u ? 0x6D2B79F5u : seed;

        if (p.normalizePeak) {
            const double currentPeak = measure(audio).peak;
            if (currentPeak <= 1e-9) {
                metadata.normalizationSkipped = true;
            } else {
                const double target = std::pow(10.0, p.targetPeakDbfs / 20.0);
                const double gain = target / currentPeak;
                for (float& sample : audio.samples)
                    sample = static_cast<float>(static_cast<double>(sample) * gain);
                metadata.normalized = true;
            }
        }
        return metadata;
    };
    std::ostringstream manifest;
    manifest << "{\n  \"schema\": \"ventusample.pack.v2\",\n  \"generator\": \"VentuSample native DSP 0.2.0\",\n  \"audio_format\": \"wav_pcm16_stereo\",\n";
    manifest << "  \"measurement_basis\": \"sample_domain\",\n";
    manifest << "  \"parameters\": {\"style\": \"" << jsonEscape(toString(p.style)) << "\", \"target\": \"" << jsonEscape(toString(p.target))
             << "\", \"bpm\": " << p.bpm << ", \"key\": \"" << noteNames[static_cast<std::size_t>(p.rootNote)]
             << "\", \"scale\": \"" << toString(p.scale) << "\", \"musical_key\": \"" << jsonEscape(selectedKeyLabel)
             << "\", \"mode_family\": \"" << modeFamily(p.scale) << "\", \"bars\": " << p.bars << ", \"variations\": " << p.variations << ", \"swing_percent\": " << p.swingPercent
             << ", \"density_percent\": " << p.densityPercent << ", \"grit_percent\": " << p.gritPercent
             << ", \"saturation_percent\": " << p.saturationPercent << ", \"stereo_percent\": " << p.stereoPercent
             << ", \"high_pass_hz\": " << p.highPassHz << ", \"low_pass_hz\": " << p.lowPassHz
             << ", \"snare_offset_ms\": " << p.snareOffsetMs << ", \"bus_compression_ratio\": " << p.busCompressionRatio
             << ", \"triplet_rolls\": " << (p.tripletRolls ? "true" : "false")
             << ", \"micro_humanize\": " << (p.microHumanize ? "true" : "false")
             << ", \"tpdf_dither\": " << (p.tpdfDither ? "true" : "false")
             << ", \"normalize_peak\": " << (p.normalizePeak ? "true" : "false")
             << ", \"target_peak_dbfs\": " << p.targetPeakDbfs
             << ", \"vinyl_dust\": " << (p.vinylDust ? "true" : "false")
             << ", \"stochastic_enabled\": " << (p.stochasticEnabled ? "true" : "false")
             << ", \"stochastic_seed\": " << p.seed
             << ", \"stochastic_intensity_percent\": " << p.stochasticIntensityPercent
             << ", \"stochastic_trigger_mode\": \""
             << (p.stochasticTriggerMode == 1 ? "bar" : (p.stochasticTriggerMode == 2 ? "onset" : "file")) << "\""
             << ", \"stochastic_plugin_mask\": " << p.stochasticPluginMask
             << ", \"stochastic_topology\": \""
             << (p.stochasticTopology == 1 ? "serial" : (p.stochasticTopology == 2 ? "parallel" : "hybrid")) << "\""
             << ", \"seed\": " << p.seed << ", \"sample_rate\": " << p.sampleRate << ", \"sampler\": \"" << toString(p.sampler)
             << "\", \"timbre\": \"" << toString(p.timbre) << "\", \"source_mix_percent\": " << p.sourceMixPercent << ", \"source_pitch_semitones\": " << p.sourcePitchSemitones
             << "},\n  \"source_sample\": \"" << jsonEscape(sourcePtr == nullptr ? std::string{} : pathFromUtf8(p.sourceSamplePath).filename().string())
             << "\",\n  \"files\": [\n";
    bool first = true;
    for (int variation = 0; variation < p.variations; ++variation) {
        if (p.target == Target::OneShotKit) {
            for (const auto* hit : kitNames) {
                const auto fileStarted = std::chrono::steady_clock::now();
                if (telemetry != nullptr)
                    telemetry->setStage(diagnostics::RenderStage::Rendering,
                                         static_cast<std::uint32_t>(variation + 1));
                auto audio = render(p, variation, hit);
                if (telemetry != nullptr) {
                    telemetry->recordBufferSamples(static_cast<std::uint64_t>(audio.samples.size()));
                    telemetry->setStage(diagnostics::RenderStage::Processing,
                                         static_cast<std::uint32_t>(variation + 1));
                }
                StochasticResult stochasticReport;
                if (!applyStochastic(audio, variation, stochasticReport)) return result;
                const bool pitchedHit = std::string(hit) == "bass" || std::string(hit) == "pluck";
                const std::string hitKeyTag = pitchedHit ? "_" + std::string(noteNames[static_cast<std::size_t>(p.rootNote)]) : "";
                const auto path = outputDirectory / (packPrefix + hitKeyTag + "_v" + std::to_string(variation + 1) + "_" + hit + ".wav");
                if (telemetry != nullptr)
                    telemetry->setStage(diagnostics::RenderStage::Exporting,
                                         static_cast<std::uint32_t>(variation + 1));
                const auto exportMetadata = prepareExport(audio, variation, path.filename().string());
                const WavExportOptions exportOptions{p.tpdfDither, exportMetadata.ditherSeed};
                std::string error;
                if (!writeWave(path, audio, error, exportOptions)) { result.error = error; return result; }
                if (telemetry != nullptr)
                    telemetry->setStage(diagnostics::RenderStage::Measuring,
                                         static_cast<std::uint32_t>(variation + 1));
                const auto stats = measure(audio);
                result.cadence.push_back(analyzeCadence(audio, p.bpm, p.swingPercent));
                result.files.push_back(path);
                if (telemetry != nullptr) {
                    const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::steady_clock::now() - fileStarted).count();
                    telemetry->recordFileCompleted(elapsed > 0 ? static_cast<std::uint64_t>(elapsed) : 0u);
                }
                if (!first) manifest << ",\n";
                first = false;
                manifest << "    {\"file\": \"" << jsonEscape(path.filename().string()) << "\", \"peak\": " << std::fixed << std::setprecision(6) << stats.peak
                         << ", \"rms\": " << stats.rms << ", \"crest_db\": " << stats.crestDb
                         << ", \"left_rms\": " << stats.leftRms << ", \"right_rms\": " << stats.rightRms
                         << ", \"stereo_correlation\": " << stats.stereoCorrelation << ", \"variation\": " << variation + 1
                         << ", \"dither\": " << (p.tpdfDither ? "\"tpdf\"" : "\"off\"")
                         << ", \"dither_seed\": " << (p.tpdfDither ? exportMetadata.ditherSeed : 0u)
                         << ", \"normalized\": " << (exportMetadata.normalized ? "true" : "false")
                         << ", \"normalization_skipped_silence\": " << (exportMetadata.normalizationSkipped ? "true" : "false")
                         << ", \"stochastic_routes\": [";
                for (std::size_t route = 0; route < stochasticReport.routes.size(); ++route) {
                    if (route != 0u) manifest << ", ";
                    manifest << "\"" << jsonEscape(stochasticReport.routes[route]) << "\"";
                }
                manifest << "], \"musical_key\": " << (pitchedHit ? "\"" + std::string(noteNames[static_cast<std::size_t>(p.rootNote)]) + "\"" : "null")
                         << ", \"scale\": null, \"mode_family\": " << (pitchedHit ? "\"pitched_note\"" : "null") << "}";
            }
        } else {
            const auto fileStarted = std::chrono::steady_clock::now();
            if (telemetry != nullptr)
                telemetry->setStage(diagnostics::RenderStage::Rendering,
                                     static_cast<std::uint32_t>(variation + 1));
            auto audio = render(p, variation, {}, sourcePtr);
            if (telemetry != nullptr) {
                telemetry->recordBufferSamples(static_cast<std::uint64_t>(audio.samples.size()));
                telemetry->setStage(diagnostics::RenderStage::Processing,
                                     static_cast<std::uint32_t>(variation + 1));
            }
            StochasticResult stochasticReport;
            if (!applyStochastic(audio, variation, stochasticReport)) return result;
            const auto path = outputDirectory / (packPrefix + "_" + slug(toString(p.target)) + "_v" + std::to_string(variation + 1) + ".wav");
            if (telemetry != nullptr)
                telemetry->setStage(diagnostics::RenderStage::Exporting,
                                     static_cast<std::uint32_t>(variation + 1));
            const auto exportMetadata = prepareExport(audio, variation, path.filename().string());
            const WavExportOptions exportOptions{p.tpdfDither, exportMetadata.ditherSeed};
            std::string error;
            if (!writeWave(path, audio, error, exportOptions)) { result.error = error; return result; }
            if (telemetry != nullptr)
                telemetry->setStage(diagnostics::RenderStage::Measuring,
                                     static_cast<std::uint32_t>(variation + 1));
            const auto stats = measure(audio);
            result.cadence.push_back(analyzeCadence(audio, p.bpm, p.swingPercent));
            result.files.push_back(path);
            if (telemetry != nullptr) {
                const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now() - fileStarted).count();
                telemetry->recordFileCompleted(elapsed > 0 ? static_cast<std::uint64_t>(elapsed) : 0u);
            }
            if (!first) manifest << ",\n";
            first = false;
            manifest << "    {\"file\": \"" << jsonEscape(path.filename().string()) << "\", \"peak\": " << std::fixed << std::setprecision(6) << stats.peak
                     << ", \"rms\": " << stats.rms << ", \"crest_db\": " << stats.crestDb
                         << ", \"left_rms\": " << stats.leftRms << ", \"right_rms\": " << stats.rightRms
                         << ", \"stereo_correlation\": " << stats.stereoCorrelation << ", \"variation\": " << variation + 1
                     << ", \"dither\": " << (p.tpdfDither ? "\"tpdf\"" : "\"off\"")
                     << ", \"dither_seed\": " << (p.tpdfDither ? exportMetadata.ditherSeed : 0u)
                     << ", \"normalized\": " << (exportMetadata.normalized ? "true" : "false")
                     << ", \"normalization_skipped_silence\": " << (exportMetadata.normalizationSkipped ? "true" : "false")
                     << ", \"stochastic_routes\": [";
            for (std::size_t route = 0; route < stochasticReport.routes.size(); ++route) {
                if (route != 0u) manifest << ", ";
                manifest << "\"" << jsonEscape(stochasticReport.routes[route]) << "\"";
            }
            manifest << "], \"musical_key\": " << (tonalTarget ? "\"" + jsonEscape(selectedKeyLabel) + "\"" : "null")
                     << ", \"scale\": " << (tonalTarget ? "\"" + jsonEscape(toString(p.scale)) + "\"" : "null")
                     << ", \"mode_family\": " << (tonalTarget ? "\"" + modeFamily(p.scale) + "\"" : "null") << "}";
        }
    }
    if (telemetry != nullptr)
        telemetry->setStage(diagnostics::RenderStage::Manifest,
                             static_cast<std::uint32_t>(p.variations));
    manifest << "\n  ]\n}\n";
    std::ofstream json(manifestPath, std::ios::binary | std::ios::trunc);
    if (!json) { result.error = "Не удалось записать manifest.json."; return result; }
    json << manifest.str();
    json.flush();
    if (!json.good()) { result.error = "Ошибка записи manifest.json."; return result; }
    return result;
}

} // namespace ventu