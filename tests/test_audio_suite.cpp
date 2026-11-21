#include "audio_suite.hpp"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

ventu::AudioSuiteBuffer makeSignal(std::size_t frames = 4096u) {
    ventu::AudioSuiteBuffer buffer;
    buffer.sampleRate = 44100;
    buffer.left.resize(frames);
    buffer.right.resize(frames);
    for (std::size_t i=0; i<frames; ++i) {
        const float t = static_cast<float>(i)/44100.0f;
        buffer.left[i] = 0.55f*std::sin(2.0f*3.14159265f*110.0f*t)
                       + 0.18f*std::sin(2.0f*3.14159265f*880.0f*t);
        buffer.right[i] = 0.52f*std::sin(2.0f*3.14159265f*110.0f*t+0.11f)
                        + 0.16f*std::sin(2.0f*3.14159265f*660.0f*t);
    }
    return buffer;
}

void testCatalog() {
    const auto& catalog = ventu::audioPluginCatalog();
    require(catalog.size() == 66u, "all 66 sound plugins must be registered");
    std::set<std::string> ids;
    for (const auto& plugin : catalog) {
        require(!plugin.id.empty() && !plugin.name.empty(), "every effect needs an ID and display name");
        require(ids.insert(plugin.id).second, "audio plugin IDs must be unique");
        require(plugin.version == "1.0.0", "every plugin must expose a semantic version");
        require(ventu::findAudioPlugin(plugin.id) != nullptr, "catalog IDs must be resolvable");
        require(ventu::findAudioPlugin(plugin.name) != nullptr, "display names must be resolvable");
    }
    require(catalog.front().id == "aetherfold", "catalog order starts with spatial processors");
    require(catalog.back().id == "sonic-catalyst", "catalog ends with the mix catalyst");
}

void testEveryDspProcessor() {
    ventu::AudioPluginParameters parameters;
    parameters.amount = 0.82f;
    parameters.wet = 0.88f;
    parameters.rate = 0.37f;
    parameters.tone = 0.61f;
    parameters.width = 0.73f;
    for (const auto& plugin : ventu::audioPluginCatalog()) {
        auto buffer = makeSignal();
        std::string error;
        const bool success = ventu::processAudioPlugin(buffer, plugin.id, parameters, error);
        require(success, "every catalogued plugin must execute");
        require(error.empty(), "successful DSP execution must clear the error");
        require(buffer.valid(), "every effect must return a finite, bounded stereo buffer");
    }
}

void testChainAndValidation() {
    auto buffer = makeSignal();
    const auto original = buffer.left;
    std::vector<ventu::AudioPluginChainItem> chain{
        {"tape-saturator-pro", {0.7f,0.65f,0.3f,0.5f,0.5f}},
        {"aetherfold", {0.6f,0.45f,0.4f,0.7f,0.7f}},
        {"low-end-lock", {0.5f,0.8f,0.2f,0.55f,0.35f}}
    };
    std::string error;
    require(ventu::processAudioChain(buffer, chain, error), "a valid effect chain must execute");
    require(buffer.valid(), "chain output must remain finite");
    bool changed = false;
    for (std::size_t i=0; i<buffer.frames(); ++i) {
        if (std::abs(buffer.left[i]-original[i]) > 1.0e-5f) { changed = true; break; }
    }
    require(changed, "the effect chain must process audio, not just return a report");

    auto unchanged = makeSignal();
    const auto before = unchanged.left;
    std::vector<ventu::AudioPluginChainItem> invalid{{"not-a-plugin", {}}};
    require(!ventu::processAudioChain(unchanged, invalid, error), "unknown plugins must reject a chain");
    require(unchanged.left == before, "failed chain processing must leave the original buffer unchanged");

    auto invalidParameters = makeSignal();
    ventu::AudioPluginParameters bad;
    bad.wet = 1.5f;
    require(!ventu::processAudioPlugin(invalidParameters, "aetherfold", bad, error),
            "out-of-range parameters must be rejected");
    require(!ventu::processAudioPlugin(invalidParameters, "unknown", {}, error),
            "unknown plugin IDs must be rejected");
}

void testWavRoundTrip() {
    const auto path = std::filesystem::temp_directory_path() / "ventu-audio-suite-roundtrip.wav";
    const auto source = makeSignal(2048u);
    ventu::AudioSuiteBuffer loaded;
    std::string error;
    require(ventu::savePcm16Wav(path, source, error), "PCM16 WAV export must succeed");
    require(ventu::loadPcm16Wav(path, loaded, error), "PCM16 WAV import must succeed");
    require(loaded.sampleRate == source.sampleRate, "WAV sample rate must survive round-trip");
    require(loaded.frames() == source.frames(), "WAV frame count must survive round-trip");
    require(loaded.valid(), "loaded WAV buffer must be valid");
    require(std::abs(loaded.left[100]-source.left[100]) < 0.0001f,
            "WAV samples must survive quantization within PCM16 precision");
    std::error_code ec;
    std::filesystem::remove(path, ec);
}
}

int main() {
    testCatalog();
    testEveryDspProcessor();
    testChainAndValidation();
    testWavRoundTrip();
    std::cout << "Ventu Audio Suite tests passed (66 DSP processors).\n";
    return 0;
}
