#include "stochastic_routing.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

ventu::AudioSuiteBuffer makeSignal(std::size_t frames = 8192u) {
    ventu::AudioSuiteBuffer buffer;
    buffer.sampleRate = 48000;
    buffer.left.resize(frames);
    buffer.right.resize(frames);
    for (std::size_t i = 0; i < frames; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(buffer.sampleRate);
        const float value = 0.55f * std::sin(2.0f * 3.14159265f * 73.0f * t)
                          + 0.19f * std::sin(2.0f * 3.14159265f * 440.0f * t);
        buffer.left[i] = value + 0.002f;
        buffer.right[i] = value * 0.96f + 0.002f;
    }
    return buffer;
}

bool samePlan(const ventu::StochasticPlan& a, const ventu::StochasticPlan& b) {
    if (a.serial.size() != b.serial.size() || a.branches.size() != b.branches.size()) return false;
    const auto sameSlot = [](const auto& x, const auto& y) {
        return x.effect.pluginId == y.effect.pluginId
            && x.effect.parameters.amount == y.effect.parameters.amount
            && x.effect.parameters.wet == y.effect.parameters.wet
            && x.effect.parameters.rate == y.effect.parameters.rate
            && x.effect.parameters.tone == y.effect.parameters.tone
            && x.effect.parameters.width == y.effect.parameters.width
            && x.locked == y.locked;
    };
    for (std::size_t i = 0; i < a.serial.size(); ++i)
        if (!sameSlot(a.serial[i], b.serial[i])) return false;
    for (std::size_t branch = 0; branch < a.branches.size(); ++branch) {
        if (a.branches[branch].size() != b.branches[branch].size()) return false;
        for (std::size_t i = 0; i < a.branches[branch].size(); ++i)
            if (!sameSlot(a.branches[branch][i], b.branches[branch][i])) return false;
    }
    return true;
}

void testDeterminismAndModes() {
    ventu::StochasticConfig config;
    config.seed = 0xA37E4ULL;
    config.mode = ventu::MutationMode::ChaosOneShot;
    std::vector<ventu::StochasticPlan> first, second;
    std::string error;
    require(ventu::buildStochasticPlans(config, {}, first, error), "chaos plan must build");
    require(ventu::buildStochasticPlans(config, {}, second, error), "same chaos plan must rebuild");
    require(first.size() == 1u && samePlan(first.front(), second.front()),
            "fixed seed must produce the same one-shot chain");

    config.mode = ventu::MutationMode::RoundRobinChain;
    config.roundRobinCount = 6;
    require(ventu::buildStochasticPlans(config, {}, first, error), "round-robin plans must build");
    require(first.size() == 6u, "round-robin mode must build the requested number of chains");

    config.mode = ventu::MutationMode::MicroVariation;
    std::vector<ventu::StochasticSlot> current{
        {{"aetherfold", {0.5f, 0.4f, 0.3f, 0.6f, 0.7f}}, false},
        {{"low-end-lock", {0.6f, 0.5f, 0.2f, 0.5f, 0.25f}}, true}
    };
    require(ventu::buildStochasticPlans(config, current, first, error), "micro-variation must build");
    require(first.size() == 1u && first.front().serial.size() >= 2u,
            "micro-variation must retain a usable chain");
    require(first.front().serial[1].effect.pluginId == "low-end-lock"
            && first.front().serial[1].effect.parameters.width == 0.25f
            && first.front().serial[1].locked,
            "locked slot ID and parameters must survive mutation unchanged");
}

void testParallelAndSafety() {
    ventu::StochasticConfig config;
    config.seed = 881122u;
    config.parallelBranches = 3u;
    config.minPlugins = 1u;
    config.maxPlugins = 3u;
    std::vector<ventu::StochasticPlan> plans;
    std::string error;
    require(ventu::buildStochasticPlans(config, {}, plans, error),
            "parallel routing plan must build");
    require(plans.size() == 1u && plans.front().branches.size() == 3u,
            "parallel plan must contain three independent branches");

    auto audio = makeSignal();
    require(ventu::processStochasticPlan(audio, plans.front(), config, error),
            "parallel stochastic processor must execute");
    require(audio.valid(), "safety output must remain finite");
    float peak = 0.0f;
    for (std::size_t i = 0; i < audio.frames(); ++i) {
        peak = std::max(peak, std::abs(audio.left[i]));
        peak = std::max(peak, std::abs(audio.right[i]));
    }
    const float ceiling = std::pow(10.0f, config.outputCeilingDb / 20.0f);
    require(peak <= ceiling + 1.0e-4f, "safety stage must enforce the configured sample-peak ceiling");

    auto unchanged = makeSignal();
    const auto before = unchanged.left;
    ventu::StochasticPlan invalid;
    require(!ventu::processStochasticPlan(unchanged, invalid, config, error),
            "empty plan must fail");
    require(unchanged.left == before, "failed plan must not mutate the caller buffer");
}

void testValidation() {
    ventu::StochasticConfig config;
    config.parallelBranches = 5u;
    std::vector<ventu::StochasticPlan> plans;
    std::string error;
    require(!ventu::buildStochasticPlans(config, {}, plans, error),
            "branch count above four must be rejected");

    config = {};
    config.pluginPool = {"not-a-real-effect"};
    require(!ventu::buildStochasticPlans(config, {}, plans, error),
            "unknown plugin IDs in a pool must be rejected");
}
}

int main() {
    testDeterminismAndModes();
    testParallelAndSafety();
    testValidation();
    std::cout << "Ventu stochastic routing tests passed.\n";
    return 0;
}
