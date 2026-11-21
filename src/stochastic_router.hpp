#pragma once

#include "generator.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ventu {

enum class StochasticEffect {
    LowPass,
    HighPass,
    Saturator,
    BitCrusher,
    Delay,
    Reverb,
    RingMod,
    StereoWidth
};

enum class StochasticTopology { Serial, Parallel, Hybrid };
enum class StochasticTriggerMode { SinglePass, FixedBlocks, ScheduledOnsets };

struct StochasticPoolEntry {
    StochasticEffect effect{StochasticEffect::Saturator};
    double weight{1.0};
    bool enabled{true};
};

std::vector<StochasticPoolEntry> defaultStochasticPool();

struct StochasticConfig {
    std::uint32_t seed{2500u};
    double intensity{0.45};
    double aggression{0.45};
    double space{0.45};
    double dirt{0.45};
    double chaos{0.45};
    std::size_t minStages{2u};
    std::size_t maxStages{5u};
    std::size_t parallelBranches{3u};
    StochasticTopology topology{StochasticTopology::Hybrid};
    StochasticTriggerMode triggerMode{StochasticTriggerMode::SinglePass};
    std::size_t blockFrames{44100u};
    double crossfadeMs{10.0};
    double maxWet{0.65};
    double maxFeedback{0.72};
    double ceilingDbfs{-0.3};
    bool autoGain{true};
    std::vector<StochasticPoolEntry> pool{defaultStochasticPool()};
};

struct StochasticStage {
    StochasticEffect effect{StochasticEffect::Saturator};
    double wet{0.0};
    double primary{0.0};
    double secondary{0.0};
    double feedback{0.0};
};

struct StochasticBranch {
    double gain{1.0};
    std::vector<StochasticStage> stages;
};

struct StochasticPlan {
    bool valid{false};
    std::string error;
    std::uint32_t seed{0u};
    StochasticTopology topology{StochasticTopology::Serial};
    std::vector<StochasticBranch> branches;
    std::size_t stageCount{0u};
    std::size_t triggerIndex{0u};
};

struct StochasticResult {
    bool success{false};
    std::string error;
    std::size_t triggerCount{0u};
    double peakBefore{0.0};
    double peakAfter{0.0};
    double rmsBefore{0.0};
    double rmsAfter{0.0};
    double maxWetUsed{0.0};
    double maxFeedbackUsed{0.0};
    std::vector<std::string> routes;
};

const char* toString(StochasticEffect effect) noexcept;
const char* toString(StochasticTopology topology) noexcept;
const char* toString(StochasticTriggerMode mode) noexcept;

StochasticPlan makeStochasticPlan(const StochasticConfig& config,
                                  std::size_t triggerIndex = 0u);
StochasticResult processStochastic(AudioBuffer& audio, const StochasticConfig& config);

} // namespace ventu
