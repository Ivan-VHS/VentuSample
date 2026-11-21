#pragma once

#include "audio_suite.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ventu {

enum class MutationMode {
    MicroVariation,
    RoundRobinChain,
    ChaosOneShot
};

struct StochasticSlot {
    AudioPluginChainItem effect;
    bool locked{false};
};

struct StochasticConfig {
    MutationMode mode{MutationMode::ChaosOneShot};
    std::uint64_t seed{1};
    std::size_t minPlugins{2};
    std::size_t maxPlugins{5};
    std::size_t roundRobinCount{4};
    std::size_t parallelBranches{0}; // 0 = serial; otherwise 2..4 branches
    float wetMinimum{0.15f};
    float wetMaximum{0.65f};
    float parameterDrift{0.12f};
    float outputCeilingDb{-0.3f};
    float maximumGainCompensationDb{6.0f};
    bool automaticGain{true};
    bool removeDc{true};
    std::vector<std::string> pluginPool; // empty means all 66 built-in DSP processors
};

struct StochasticPlan {
    std::uint64_t seed{0};
    std::vector<StochasticSlot> serial;
    std::vector<std::vector<StochasticSlot>> branches;
};

// Builds deterministic plans. Locked slots retain their exact position and parameters.
// Round-robin mode returns 4..8 precomputed plans; other modes return one plan.
bool buildStochasticPlans(const StochasticConfig& config,
                          const std::vector<StochasticSlot>& currentSlots,
                          std::vector<StochasticPlan>& plans,
                          std::string& error);

// Offline transactional renderer with bounded branch summing, RMS matching,
// DC blocking and a sample-peak ceiling. No live feedback graph is created.
bool processStochasticPlan(AudioSuiteBuffer& buffer,
                           const StochasticPlan& plan,
                           const StochasticConfig& config,
                           std::string& error);

} // namespace ventu
