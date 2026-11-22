#include "quantum_audio.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>
#include <string>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

ventu::AudioBuffer fixture() {
    ventu::AudioBuffer audio;
    audio.sampleRate = 48000;
    audio.channels = 2;
    constexpr std::size_t frames = 4096u;
    audio.samples.resize(frames * 2u);
    for (std::size_t frame = 0; frame < frames; ++frame) {
        const double time = static_cast<double>(frame) / audio.sampleRate;
        const double low = 0.16 * std::sin(2.0 * 3.141592653589793 * 43.0 * time);
        const double mid = 0.11 * std::sin(2.0 * 3.141592653589793 * 440.0 * time);
        const double transient = frame == 96u ? 0.72 : 0.0;
        audio.samples[frame * 2u] = static_cast<float>(low + mid + transient);
        audio.samples[frame * 2u + 1u] = static_cast<float>(low + mid * 0.91 + transient);
    }
    return audio;
}

void testCatalogue() {
    const auto& plugins = ventu::quantumAudioPlugins();
    require(plugins.size() == 133u, "the catalogue must contain exactly 133 modules");
    std::set<std::string> ids;
    std::set<std::string> names;
    std::set<std::string> categories;
    for (const auto& plugin : plugins) {
        require(!plugin.id.empty() && !plugin.name.empty(), "every module needs a stable ID and name");
        require(ids.insert(std::string(plugin.id)).second, "module IDs must be unique");
        require(names.insert(std::string(plugin.name)).second, "module names must be unique");
        categories.insert(std::string(ventu::quantumAudioCategoryName(plugin.category)));
        require(plugin.defaults.amount >= 0.0 && plugin.defaults.amount <= 1.0,
                "default amount must be in the normalized range");
        require(plugin.defaults.tone >= 0.0 && plugin.defaults.tone <= 1.0,
                "default tone must be in the normalized range");
        require(plugin.defaults.rate >= 0.0 && plugin.defaults.rate <= 1.0,
                "default rate must be in the normalized range");
        require(plugin.defaults.mix >= 0.0 && plugin.defaults.mix <= 1.0,
                "default mix must be in the normalized range");
    }
    require(categories.size() == 5u, "all five module families must be represented");
    require(ventu::findQuantumAudioPlugin("QUARK-CONFINER") != nullptr,
            "module lookup must be case-insensitive");
    require(ventu::findQuantumAudioPlugin("omega-field") != nullptr,
            "the final catalogue module must be registered");
    require(ventu::findQuantumAudioPlugin("not-a-module") == nullptr,
            "unknown module lookup must return null");
}

void testEveryModuleProcessesFiniteAudio() {
    const auto& plugins = ventu::quantumAudioPlugins();
    for (const auto& plugin : plugins) {
        auto audio = fixture();
        const auto sizeBefore = audio.samples.size();
        const auto result = ventu::processQuantumAudio(plugin.id, audio, plugin.defaults);
        if (!result.success) {
            std::cerr << "FAILED module " << plugin.id << ": " << result.message << '\n';
            std::exit(EXIT_FAILURE);
        }
        require(audio.samples.size() == sizeBefore, "processors must preserve the input frame count");
        require(audio.channels == 2 && audio.sampleRate == 48000,
                "processors must preserve audio format metadata");
        for (const float sample : audio.samples)
            require(std::isfinite(sample), "processed audio must never contain NaN or infinity");
        const auto stats = ventu::measure(audio);
        require(stats.peak <= 8.0, "processor output must remain inside the internal safety bound");
        require(stats.rms >= 0.0, "RMS measurement must be non-negative");
    }
}

void testDryMixAndValidation() {
    auto dry = fixture();
    const auto original = dry.samples;
    ventu::QuantumAudioParameters bypass{0.85, 0.9, 0.8, 0.0};
    const auto bypassed = ventu::processQuantumAudio("strange-quark", dry, bypass);
    require(bypassed.success, "dry mix must be a valid bypass");
    require(dry.samples == original, "zero wet mix must preserve every sample exactly");

    auto invalid = fixture();
    const auto before = invalid.samples;
    ventu::QuantumAudioParameters bad{std::numeric_limits<double>::quiet_NaN(), 0.5, 0.5, 0.5};
    const auto rejected = ventu::processQuantumAudio("omega-field", invalid, bad);
    require(!rejected.success, "non-finite controls must be rejected");
    require(invalid.samples == before, "invalid controls must not mutate the input");

    auto malformed = fixture();
    malformed.channels = 0;
    const auto malformedResult = ventu::processQuantumAudio("quark-bass", malformed, {});
    require(!malformedResult.success, "invalid channel counts must be rejected");

    auto contaminated = fixture();
    contaminated.samples[12] = std::numeric_limits<float>::infinity();
    const auto contaminatedResult = ventu::processQuantumAudio("quantum-pad", contaminated, {});
    require(!contaminatedResult.success, "non-finite input samples must be rejected");
}

void testAnalyzerIsReadOnly() {
    auto audio = fixture();
    const auto original = audio.samples;
    const auto result = ventu::processQuantumAudio("coherence-meter", audio, {});
    require(result.success, "coherence meter must analyze valid input");
    require(audio.samples == original, "analysis-only module must not modify audio");
    require(result.before.peak > 0.0 && result.after.peak > 0.0,
            "analyzer report must include signal measurements");
}
}

int main() {
    testCatalogue();
    testEveryModuleProcessesFiniteAudio();
    testDryMixAndValidation();
    testAnalyzerIsReadOnly();
    std::cout << "Ventu Quantum Audio Engine tests passed (133 modules).\n";
    return 0;
}
