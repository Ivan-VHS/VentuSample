#include "stochastic_routing.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <unordered_set>
#include <utility>

namespace ventu {
namespace {

float clamp01(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float mutateValue(float value, float depth, std::mt19937_64& rng) {
    std::uniform_real_distribution<float> delta(-depth, depth);
    return clamp01(value + delta(rng));
}

AudioPluginParameters randomParameters(const StochasticConfig& config,
                                       std::mt19937_64& rng) {
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);
    std::uniform_real_distribution<float> wet(config.wetMinimum, config.wetMaximum);
    return {
        0.25f + unit(rng) * 0.70f,
        wet(rng),
        0.10f + unit(rng) * 0.80f,
        0.12f + unit(rng) * 0.76f,
        0.18f + unit(rng) * 0.72f
    };
}

std::vector<const AudioPluginDescriptor*> resolvePool(const StochasticConfig& config,
                                                        std::string& error) {
    std::vector<const AudioPluginDescriptor*> pool;
    const auto& catalog = audioPluginCatalog();
    if (config.pluginPool.empty()) {
        for (const auto& descriptor : catalog) pool.push_back(&descriptor);
        return pool;
    }

    std::unordered_set<std::string> seen;
    for (const auto& id : config.pluginPool) {
        const auto* descriptor = findAudioPlugin(id);
        if (!descriptor) {
            error = "Unknown plugin in stochastic pool: " + id;
            return {};
        }
        if (seen.insert(descriptor->id).second) pool.push_back(descriptor);
    }
    return pool;
}

std::size_t randomCount(const StochasticConfig& config, std::mt19937_64& rng) {
    std::uniform_int_distribution<std::size_t> count(config.minPlugins, config.maxPlugins);
    return count(rng);
}

std::vector<StochasticSlot> makeChain(const StochasticConfig& config,
                                      const std::vector<StochasticSlot>& current,
                                      const std::vector<const AudioPluginDescriptor*>& pool,
                                      std::mt19937_64& rng,
                                      bool keepCurrentIds) {
    std::size_t target = randomCount(config, rng);
    for (std::size_t i = 0; i < current.size(); ++i) {
        if (current[i].locked) target = std::max(target, i + 1u);
    }
    target = std::min<std::size_t>(target, 12u);

    std::vector<StochasticSlot> result(target);
    std::unordered_set<std::string> used;
    std::uniform_int_distribution<std::size_t> choose(0u, pool.size() - 1u);

    for (std::size_t i = 0; i < target; ++i) {
        if (i < current.size() && current[i].locked) {
            result[i] = current[i];
            used.insert(result[i].effect.pluginId);
            continue;
        }

        if (keepCurrentIds && i < current.size()
            && findAudioPlugin(current[i].effect.pluginId) != nullptr) {
            result[i] = current[i];
            result[i].locked = false;
            auto& p = result[i].effect.parameters;
            p.amount = mutateValue(p.amount, config.parameterDrift, rng);
            p.wet = std::clamp(mutateValue(p.wet, config.parameterDrift, rng),
                               config.wetMinimum, config.wetMaximum);
            p.rate = mutateValue(p.rate, config.parameterDrift, rng);
            p.tone = mutateValue(p.tone, config.parameterDrift, rng);
            p.width = mutateValue(p.width, config.parameterDrift, rng);
            used.insert(result[i].effect.pluginId);
            continue;
        }

        std::size_t attempts = 0;
        const AudioPluginDescriptor* selected = nullptr;
        while (attempts++ < pool.size() * 2u) {
            const auto* candidate = pool[choose(rng)];
            if (used.find(candidate->id) == used.end()) {
                selected = candidate;
                break;
            }
        }
        if (!selected) selected = pool[choose(rng)];
        result[i].effect.pluginId = selected->id;
        result[i].effect.parameters = randomParameters(config, rng);
        result[i].locked = false;
        used.insert(selected->id);
    }
    return result;
}

std::vector<AudioPluginChainItem> asChain(const std::vector<StochasticSlot>& slots) {
    std::vector<AudioPluginChainItem> result;
    result.reserve(slots.size());
    for (const auto& slot : slots) result.push_back(slot.effect);
    return result;
}

double rmsOf(const AudioSuiteBuffer& buffer) {
    if (buffer.left.empty()) return 0.0;
    long double sum = 0.0L;
    for (std::size_t i = 0; i < buffer.frames(); ++i) {
        const long double l = buffer.left[i];
        const long double r = buffer.right[i];
        sum += l * l + r * r;
    }
    const long double count = static_cast<long double>(buffer.frames()) * 2.0L;
    return std::sqrt(static_cast<double>(sum / count));
}

void applyDcBlocker(AudioSuiteBuffer& buffer) {
    const double r = std::exp(-2.0 * 3.14159265358979323846 * 18.0
                              / static_cast<double>(buffer.sampleRate));
    float previousInputL = 0.0f, previousInputR = 0.0f;
    float previousOutputL = 0.0f, previousOutputR = 0.0f;
    for (std::size_t i = 0; i < buffer.frames(); ++i) {
        const float inputL = buffer.left[i];
        const float inputR = buffer.right[i];
        const float outputL = inputL - previousInputL + static_cast<float>(r) * previousOutputL;
        const float outputR = inputR - previousInputR + static_cast<float>(r) * previousOutputR;
        buffer.left[i] = outputL;
        buffer.right[i] = outputR;
        previousInputL = inputL;
        previousInputR = inputR;
        previousOutputL = outputL;
        previousOutputR = outputR;
    }
}

void applyPeakCeiling(AudioSuiteBuffer& buffer, float ceiling) {
    float peak = 0.0f;
    for (std::size_t i = 0; i < buffer.frames(); ++i) {
        peak = std::max(peak, std::abs(buffer.left[i]));
        peak = std::max(peak, std::abs(buffer.right[i]));
    }
    if (peak > ceiling && peak > 0.0f) {
        const float gain = ceiling / peak;
        for (std::size_t i = 0; i < buffer.frames(); ++i) {
            buffer.left[i] *= gain;
            buffer.right[i] *= gain;
        }
    }
}

bool validConfig(const StochasticConfig& config, std::string& error) {
    if (config.minPlugins < 1u || config.maxPlugins < config.minPlugins
        || config.maxPlugins > 12u) {
        error = "Plugin count must be within 1..12 and maxPlugins must not be below minPlugins.";
        return false;
    }
    if (config.parallelBranches != 0u
        && (config.parallelBranches < 2u || config.parallelBranches > 4u)) {
        error = "Parallel split must use 2..4 branches, or zero for serial routing.";
        return false;
    }
    if (config.mode == MutationMode::RoundRobinChain
        && (config.roundRobinCount < 4u || config.roundRobinCount > 8u)) {
        error = "Round-robin mode requires 4..8 precomputed chains.";
        return false;
    }
    if (!std::isfinite(config.wetMinimum) || !std::isfinite(config.wetMaximum)
        || config.wetMinimum < 0.0f || config.wetMaximum > 1.0f
        || config.wetMinimum > config.wetMaximum) {
        error = "Wet bounds must be finite values in ascending order within 0..1.";
        return false;
    }
    if (!std::isfinite(config.parameterDrift) || config.parameterDrift < 0.0f
        || config.parameterDrift > 0.5f) {
        error = "Parameter drift must be within 0..0.5.";
        return false;
    }
    if (!std::isfinite(config.outputCeilingDb) || config.outputCeilingDb > -0.1f
        || config.outputCeilingDb < -12.0f) {
        error = "Output ceiling must be between -12 and -0.1 dBFS.";
        return false;
    }
    if (!std::isfinite(config.maximumGainCompensationDb)
        || config.maximumGainCompensationDb < 0.0f
        || config.maximumGainCompensationDb > 12.0f) {
        error = "Automatic gain compensation must be between 0 and 12 dB.";
        return false;
    }
    return true;
}

} // namespace

bool buildStochasticPlans(const StochasticConfig& config,
                          const std::vector<StochasticSlot>& currentSlots,
                          std::vector<StochasticPlan>& plans,
                          std::string& error) {
    plans.clear();
    error.clear();
    if (!validConfig(config, error)) return false;

    const auto pool = resolvePool(config, error);
    if (!error.empty()) return false;
    if (pool.empty()) {
        error = "Stochastic plugin pool is empty.";
        return false;
    }

    const std::size_t planCount = config.mode == MutationMode::RoundRobinChain
        ? config.roundRobinCount : 1u;
    plans.reserve(planCount);

    for (std::size_t index = 0; index < planCount; ++index) {
        const std::uint64_t derivedSeed = config.seed
            ^ (0x9e3779b97f4a7c15ULL * static_cast<std::uint64_t>(index + 1u));
        std::mt19937_64 rng(derivedSeed);
        StochasticPlan plan;
        plan.seed = derivedSeed;

        const bool preserveCurrent = config.mode == MutationMode::MicroVariation;
        if (config.parallelBranches == 0u) {
            plan.serial = makeChain(config, currentSlots, pool, rng, preserveCurrent);
        } else {
            plan.branches.reserve(config.parallelBranches);
            for (std::size_t branch = 0; branch < config.parallelBranches; ++branch) {
                const auto branchSeed = derivedSeed
                    ^ (0xd1b54a32d192ed03ULL * static_cast<std::uint64_t>(branch + 1u));
                std::mt19937_64 branchRng(branchSeed);
                plan.branches.push_back(makeChain(config, currentSlots, pool,
                                                   branchRng, preserveCurrent));
            }
        }
        plans.push_back(std::move(plan));
    }

    return true;
}

bool processStochasticPlan(AudioSuiteBuffer& buffer,
                           const StochasticPlan& plan,
                           const StochasticConfig& config,
                           std::string& error) {
    error.clear();
    if (!buffer.valid()) {
        error = "Stochastic input buffer is invalid.";
        return false;
    }
    if (!validConfig(config, error)) return false;
    if (plan.serial.empty() && plan.branches.empty()) {
        error = "Stochastic plan contains no processing slots.";
        return false;
    }

    const AudioSuiteBuffer original = buffer;
    AudioSuiteBuffer rendered = original;

    if (!plan.branches.empty()) {
        if (plan.branches.size() < 2u || plan.branches.size() > 4u) {
            error = "Stochastic plan branch count must be 2..4.";
            return false;
        }
        std::vector<AudioSuiteBuffer> branchBuffers;
        branchBuffers.reserve(plan.branches.size());
        for (const auto& branch : plan.branches) {
            if (branch.empty()) {
                error = "Stochastic plan contains an empty parallel branch.";
                return false;
            }
            auto output = original;
            if (!processAudioChain(output, asChain(branch), error)) return false;
            branchBuffers.push_back(std::move(output));
        }

        const float dryGain = 0.30f;
        const float branchGain = 0.70f / std::sqrt(static_cast<float>(branchBuffers.size()));
        for (std::size_t i = 0; i < rendered.frames(); ++i) {
            float left = dryGain * original.left[i];
            float right = dryGain * original.right[i];
            for (const auto& branch : branchBuffers) {
                left += branchGain * branch.left[i];
                right += branchGain * branch.right[i];
            }
            rendered.left[i] = left;
            rendered.right[i] = right;
        }
    } else {
        if (!processAudioChain(rendered, asChain(plan.serial), error)) return false;
    }

    if (config.automaticGain) {
        const double inputRms = rmsOf(original);
        const double outputRms = rmsOf(rendered);
        if (inputRms > 1.0e-9 && outputRms > 1.0e-9) {
            const double maximumGain = std::pow(10.0,
                static_cast<double>(config.maximumGainCompensationDb) / 20.0);
            const double gain = std::clamp(inputRms / outputRms,
                                           1.0 / maximumGain, maximumGain);
            for (std::size_t i = 0; i < rendered.frames(); ++i) {
                rendered.left[i] = static_cast<float>(rendered.left[i] * gain);
                rendered.right[i] = static_cast<float>(rendered.right[i] * gain);
            }
        }
    }

    if (config.removeDc) applyDcBlocker(rendered);
    const float ceiling = std::pow(10.0f, config.outputCeilingDb / 20.0f);
    applyPeakCeiling(rendered, ceiling);
    if (!rendered.valid()) {
        error = "Stochastic safety stage produced an invalid audio buffer.";
        return false;
    }

    buffer = std::move(rendered);
    return true;
}

} // namespace ventu
