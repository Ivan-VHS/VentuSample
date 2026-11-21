#include "stochastic_router.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

ventu::AudioBuffer makeSignal(std::size_t frames = 44100u) {
    ventu::AudioBuffer audio;
    audio.sampleRate = 44100;
    audio.channels = 2;
    audio.samples.resize(frames * 2u);
    for (std::size_t frame = 0; frame < frames; ++frame) {
        const double time = static_cast<double>(frame) / 44100.0;
        const float left = static_cast<float>(0.46 * std::sin(2.0 * 3.141592653589793 * 110.0 * time));
        const float right = static_cast<float>(0.39 * std::sin(2.0 * 3.141592653589793 * 164.81 * time));
        audio.samples[frame * 2u] = left;
        audio.samples[frame * 2u + 1u] = right;
    }
    return audio;
}

void testDeterministicProcessing() {
    auto first = makeSignal();
    auto second = first;
    ventu::StochasticConfig config;
    config.seed = 20261121u;
    config.intensity = 0.72;
    config.minStages = 4u;
    config.maxStages = 6u;
    config.triggerMode = ventu::StochasticTriggerMode::FixedBlocks;
    config.blockFrames = 11025u;

    const auto a = ventu::processStochastic(first, config);
    const auto b = ventu::processStochastic(second, config);
    require(a.success && b.success, "valid stereo buffers should process successfully");
    require(first.samples == second.samples, "fixed seed and settings must be bit-repeatable");
    require(first.samples.size() == makeSignal().samples.size(), "processing must preserve sample count");
    require(first.samples != makeSignal().samples, "enabled stochastic processing must alter the source");
    require(a.triggerCount == 4u, "fixed-block trigger should mutate once per block");
    require(!a.routes.empty(), "processed route should be reported");
    require(a.peakAfter <= 0.967, "master sample-peak guard must enforce the -0.3 dBFS ceiling");
    require(a.rmsAfter > 0.0 && std::isfinite(a.rmsAfter), "RMS output must stay finite and non-zero");
}

void testPlanAndParallelBranches() {
    ventu::StochasticConfig serial;
    serial.seed = 77u;
    serial.minStages = 4u;
    serial.maxStages = 4u;
    serial.topology = ventu::StochasticTopology::Serial;
    const auto serialPlan = ventu::makeStochasticPlan(serial);
    require(serialPlan.valid, "serial route should be generated");
    require(serialPlan.topology == ventu::StochasticTopology::Serial,
            "serial configuration must yield one serial branch");
    require(serialPlan.branches.size() == 1u && serialPlan.stageCount == 4u,
            "serial route should contain the requested four stages");

    auto parallel = serial;
    parallel.topology = ventu::StochasticTopology::Parallel;
    parallel.parallelBranches = 3u;
    const auto parallelPlan = ventu::makeStochasticPlan(parallel);
    require(parallelPlan.valid, "parallel route should be generated");
    require(parallelPlan.topology == ventu::StochasticTopology::Parallel,
            "parallel configuration must create a split graph");
    require(parallelPlan.branches.size() == 3u,
            "parallel topology should create three bounded branches");
    std::size_t stageCount = 0u;
    for (const auto& branch : parallelPlan.branches) stageCount += branch.stages.size();
    require(stageCount == parallelPlan.stageCount, "parallel branch distribution must preserve all stages");
}

void testSafeParameterBoundsAndPoolValidation() {
    ventu::StochasticConfig config;
    config.seed = 91u;
    config.topology = ventu::StochasticTopology::Serial;
    config.minStages = 8u;
    config.maxStages = 8u;
    config.pool = {{ventu::StochasticEffect::Delay, 1.0, true}};
    const auto delayPlan = ventu::makeStochasticPlan(config);
    require(delayPlan.valid, "a one-plugin pool should be valid with one stage");
    for (const auto& branch : delayPlan.branches) {
        for (const auto& stage : branch.stages) {
            require(stage.feedback <= 0.72, "feedback must never exceed the global safety ceiling");
            require(stage.wet <= 0.65, "wet amount must never exceed the global safety ceiling");
        }
    }

    config.pool = {{ventu::StochasticEffect::Delay, 0.0, true},
                   {ventu::StochasticEffect::Reverb, -1.0, true}};
    const auto emptyPool = ventu::makeStochasticPlan(config);
    require(!emptyPool.valid, "empty or invalid weighted pools must be rejected");

    config.pool = ventu::defaultStochasticPool();
    config.minStages = 0u;
    const auto invalidBounds = ventu::makeStochasticPlan(config);
    require(!invalidBounds.valid, "invalid stage ranges must be rejected");
}

void testScheduledOnsetsAndInputValidation() {
    auto audio = makeSignal(22050u);
    audio.eventFrames = {0u, 5000u, 10000u, 15000u, 20000u};
    ventu::StochasticConfig config;
    config.triggerMode = ventu::StochasticTriggerMode::ScheduledOnsets;
    config.minStages = 1u;
    config.maxStages = 3u;
    const auto report = ventu::processStochastic(audio, config);
    require(report.success, "scheduled musical onset triggers should process");
    require(report.triggerCount >= 3u, "separated scheduled onsets should create route mutations");

    auto invalid = makeSignal(128u);
    invalid.channels = 1;
    const auto rejected = ventu::processStochastic(invalid, config);
    require(!rejected.success, "non-stereo buffer should be rejected");
}

void testZeroIntensityIsTransparent() {
    auto audio = makeSignal(4096u);
    const auto original = audio.samples;
    ventu::StochasticConfig config;
    config.intensity = 0.0;
    const auto result = ventu::processStochastic(audio, config);
    require(result.success, "zero-intensity bypass should succeed");
    require(audio.samples == original, "zero intensity must be a bit-transparent bypass");
}

} // namespace

int main() {
    testDeterministicProcessing();
    testPlanAndParallelBranches();
    testSafeParameterBoundsAndPoolValidation();
    testScheduledOnsetsAndInputValidation();
    testZeroIntensityIsTransparent();
    std::cout << "VentuSample stochastic routing tests passed.\n";
    return 0;
}
