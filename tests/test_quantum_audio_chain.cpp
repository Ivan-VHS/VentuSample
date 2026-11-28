#include "quantum_audio.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
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
    audio.samples.resize(1024u * 2u);
    for (std::size_t frame = 0; frame < 1024u; ++frame) {
        const float sample = static_cast<float>((frame % 64u) / 128.0);
        audio.samples[frame * 2u] = sample;
        audio.samples[frame * 2u + 1u] = sample * 0.9f;
    }
    return audio;
}

void testSuccessfulOrderedChain() {
    auto audio = fixture();
    const auto original = audio.samples;
    const std::vector<ventu::QuantumAudioChainStep> chain{
        {"strange-quark", {0.62, 0.78, 0.24, 0.71}},
        {"pion-punch", {0.78, 0.38, 0.15, 0.82}},
        {"coherence-meter", {}},
    };
    const auto report = ventu::processQuantumAudioChain(chain, audio);
    require(report.success, "valid DSP chain should complete");
    require(report.completedSteps == chain.size(), "all requested steps should complete");
    require(report.processedPluginIds.size() == chain.size(), "execution order should be preserved");
    require(report.processedPluginIds[0] == "strange-quark", "first plugin should run first");
    require(report.processedPluginIds[1] == "pion-punch", "second plugin should run second");
    require(report.processedPluginIds[2] == "coherence-meter", "analyzer should remain in the chain");
    require(audio.samples != original, "successful chain should commit the rendered samples");
    require(report.after.peak >= 0.0, "final signal statistics should be reported");
}

void testFailureDoesNotMutateCallerBuffer() {
    auto audio = fixture();
    const auto original = audio.samples;
    const std::vector<ventu::QuantumAudioChainStep> chain{
        {"strange-quark", {0.62, 0.78, 0.24, 0.71}},
        {"not-a-real-module", {}},
    };
    const auto report = ventu::processQuantumAudioChain(chain, audio);
    require(!report.success, "unknown plugin should fail the chain");
    require(report.completedSteps == 1u, "report should identify the completed prefix");
    require(report.failedStepId == "not-a-real-module", "report should identify the failed step");
    require(audio.samples == original, "failed chain must leave caller audio unchanged");
}

void testInvalidControlsDoNotCommitEarlierSteps() {
    auto audio = fixture();
    const auto original = audio.samples;
    const std::vector<ventu::QuantumAudioChainStep> chain{
        {"strange-quark", {0.62, 0.78, 0.24, 0.71}},
        {"pion-punch", {std::numeric_limits<double>::quiet_NaN(), 0.38, 0.15, 0.82}},
    };
    const auto report = ventu::processQuantumAudioChain(chain, audio);
    require(!report.success, "invalid module controls should stop the chain");
    require(report.completedSteps == 1u, "successful prefix length should be retained");
    require(audio.samples == original, "invalid later step must roll back earlier processing");
}

void testEmptyAndOversizedChains() {
    auto audio = fixture();
    const auto original = audio.samples;
    const std::vector<ventu::QuantumAudioChainStep> empty;
    const auto emptyReport = ventu::processQuantumAudioChain(empty, audio);
    require(emptyReport.success, "empty chain should be a valid no-op");
    require(audio.samples == original, "empty chain must not change the source");

    std::vector<ventu::QuantumAudioChainStep> tooMany(
        ventu::quantumAudioPlugins().size() + 1u,
        {"strange-quark", {}}
    );
    const auto rejected = ventu::processQuantumAudioChain(tooMany, audio);
    require(!rejected.success, "chain longer than the catalogue must be rejected");
    require(audio.samples == original, "oversized chain must not change the source");
}
}

int main() {
    testSuccessfulOrderedChain();
    testFailureDoesNotMutateCallerBuffer();
    testInvalidControlsDoNotCommitEarlierSteps();
    testEmptyAndOversizedChains();
    std::cout << "VentuSample syntax-neutral DSP chain tests passed.\n";
    return 0;
}
