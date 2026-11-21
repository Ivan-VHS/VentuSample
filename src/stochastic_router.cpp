#include "stochastic_router.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>
#include <utility>

namespace ventu {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kMaxFeedback = 0.72;
constexpr double kMaxWet = 0.65;
constexpr double kDefaultCeiling = 0.96605087899;

double unit(double value) {
    return std::clamp(value, 0.0, 1.0);
}

double finiteOr(double value, double fallback) {
    return std::isfinite(value) ? value : fallback;
}

double randomBetween(std::mt19937& rng, double low, double high) {
    std::uniform_real_distribution<double> distribution(low, high);
    return distribution(rng);
}

std::size_t randomIndex(std::mt19937& rng, std::size_t upperExclusive) {
    if (upperExclusive <= 1u) return 0u;
    std::uniform_int_distribution<std::size_t> distribution(0u, upperExclusive - 1u);
    return distribution(rng);
}

std::uint32_t mixedSeed(std::uint32_t seed, std::size_t index) {
    std::uint32_t value = seed ^ (static_cast<std::uint32_t>(index) * 0x9E3779B9u);
    value ^= value >> 16u;
    value *= 0x7FEB352Du;
    value ^= value >> 15u;
    value *= 0x846CA68Bu;
    value ^= value >> 16u;
    return value;
}

StochasticStage makeStage(StochasticEffect effect, const StochasticConfig& config,
                          std::mt19937& rng) {
    const double intensity = unit(finiteOr(config.intensity, 0.45));
    const double aggression = unit(finiteOr(config.aggression, 0.45));
    const double space = unit(finiteOr(config.space, 0.45));
    const double dirt = unit(finiteOr(config.dirt, 0.45));
    const double chaos = unit(finiteOr(config.chaos, 0.45));
    const double maxWet = std::clamp(finiteOr(config.maxWet, kMaxWet), 0.0, kMaxWet);
    const double maxFeedback = std::clamp(
        finiteOr(config.maxFeedback, kMaxFeedback), 0.0, kMaxFeedback);

    StochasticStage stage;
    stage.effect = effect;
    stage.wet = std::clamp((0.10 + space * 0.24 + chaos * 0.16
                            + randomBetween(rng, 0.02, 0.16)) * intensity,
                           0.0, maxWet);

    switch (effect) {
    case StochasticEffect::LowPass:
        stage.primary = std::clamp(
            900.0 + std::pow(1.0 - aggression, 2.0) * 15000.0
                * randomBetween(rng, 0.75, 1.10),
            500.0, 0.45 * 192000.0);
        break;
    case StochasticEffect::HighPass:
        stage.primary = std::clamp(
            25.0 + aggression * 900.0 * randomBetween(rng, 0.65, 1.15),
            20.0, 0.35 * 192000.0);
        break;
    case StochasticEffect::Saturator:
        stage.primary = std::clamp(
            1.0 + aggression * 9.0 + dirt * 2.0
                + randomBetween(rng, 0.1, 1.5),
            1.0, 12.0);
        break;
    case StochasticEffect::BitCrusher:
        stage.primary = std::clamp(
            6.0 + (1.0 - dirt) * 8.0 + randomBetween(rng, -1.0, 1.0),
            5.0, 16.0);
        stage.secondary = std::clamp(
            1.0 + dirt * 10.0 + chaos * 2.0, 1.0, 16.0);
        break;
    case StochasticEffect::Delay:
        stage.primary = std::clamp(
            18.0 + space * 280.0 * randomBetween(rng, 0.65, 1.15),
            8.0, 480.0);
        stage.feedback = std::clamp(
            (0.12 + space * 0.38 + chaos * 0.12)
                * randomBetween(rng, 0.75, 1.0),
            0.0, maxFeedback);
        break;
    case StochasticEffect::Reverb:
        stage.primary = std::clamp(
            70.0 + space * 520.0 * randomBetween(rng, 0.75, 1.15),
            40.0, 700.0);
        stage.feedback = std::clamp(
            (0.18 + space * 0.38) * randomBetween(rng, 0.75, 1.0),
            0.0, maxFeedback);
        break;
    case StochasticEffect::RingMod:
        stage.primary = std::clamp(
            24.0 + chaos * 230.0 * randomBetween(rng, 0.75, 1.15),
            12.0, 900.0);
        break;
    case StochasticEffect::StereoWidth:
        stage.primary = std::clamp(
            0.55 + space * 1.05 * randomBetween(rng, 0.85, 1.10),
            0.25, 1.8);
        break;
    }
    return stage;
}

void applyStage(AudioBuffer& audio, const StochasticStage& stage) {
    if (audio.samples.empty() || audio.channels != 2 || stage.wet <= 0.0) return;

    const std::size_t frames = audio.frames();
    const std::size_t channels = 2u;
    const double sampleRate = static_cast<double>(audio.sampleRate);
    const std::vector<float> dry = audio.samples;
    std::vector<float> wetSamples = dry;

    switch (stage.effect) {
    case StochasticEffect::LowPass: {
        const double cutoff = std::clamp(stage.primary, 20.0, sampleRate * 0.45);
        const double alpha = 1.0 - std::exp(-2.0 * kPi * cutoff / sampleRate);
        std::array<double, 2> state{0.0, 0.0};
        for (std::size_t frame = 0; frame < frames; ++frame) {
            for (std::size_t channel = 0; channel < channels; ++channel) {
                const std::size_t index = frame * channels + channel;
                state[channel] += alpha * (static_cast<double>(dry[index]) - state[channel]);
                wetSamples[index] = static_cast<float>(state[channel]);
            }
        }
        break;
    }
    case StochasticEffect::HighPass: {
        const double cutoff = std::clamp(stage.primary, 20.0, sampleRate * 0.35);
        const double alpha = std::exp(-2.0 * kPi * cutoff / sampleRate);
        std::array<double, 2> previousInput{0.0, 0.0};
        std::array<double, 2> previousOutput{0.0, 0.0};
        for (std::size_t frame = 0; frame < frames; ++frame) {
            for (std::size_t channel = 0; channel < channels; ++channel) {
                const std::size_t index = frame * channels + channel;
                const double input = static_cast<double>(dry[index]);
                const double output = alpha * (previousOutput[channel] + input - previousInput[channel]);
                previousInput[channel] = input;
                previousOutput[channel] = output;
                wetSamples[index] = static_cast<float>(output);
            }
        }
        break;
    }
    case StochasticEffect::Saturator: {
        const double drive = std::clamp(stage.primary, 1.0, 12.0);
        const double normalization = std::max(1.0e-9, std::tanh(drive));
        for (std::size_t i = 0; i < wetSamples.size(); ++i) {
            wetSamples[i] = static_cast<float>(
                std::tanh(static_cast<double>(dry[i]) * drive) / normalization);
        }
        break;
    }
    case StochasticEffect::BitCrusher: {
        const int bits = static_cast<int>(std::clamp(std::lround(stage.primary), 5l, 16l));
        const std::size_t holdFrames = static_cast<std::size_t>(
            std::clamp(std::lround(stage.secondary), 1l, 16l));
        const double levels = std::pow(2.0, static_cast<double>(bits - 1)) - 1.0;
        std::array<float, 2> held{0.0f, 0.0f};
        for (std::size_t frame = 0; frame < frames; ++frame) {
            if ((frame % holdFrames) == 0u) {
                for (std::size_t channel = 0; channel < channels; ++channel) {
                    const auto index = frame * channels + channel;
                    held[channel] = static_cast<float>(
                        std::round(static_cast<double>(dry[index]) * levels) / levels);
                }
            }
            for (std::size_t channel = 0; channel < channels; ++channel)
                wetSamples[frame * channels + channel] = held[channel];
        }
        break;
    }
    case StochasticEffect::Delay: {
        const auto requested = static_cast<std::size_t>(
            std::max(1.0, std::round(stage.primary * sampleRate / 1000.0)));
        const std::size_t frameCap = frames > 1u ? frames - 1u : 1u;
        const std::size_t delayFrames = std::min(
            requested, std::max<std::size_t>(1u, std::min(frameCap,
                static_cast<std::size_t>(sampleRate * 0.45))));
        const double feedback = std::clamp(stage.feedback, 0.0, kMaxFeedback);
        std::vector<float> buffer(delayFrames * channels, 0.0f);
        for (std::size_t frame = 0; frame < frames; ++frame) {
            const std::size_t position = frame % delayFrames;
            for (std::size_t channel = 0; channel < channels; ++channel) {
                const std::size_t index = frame * channels + channel;
                const std::size_t delayIndex = channel * delayFrames + position;
                const double delayed = static_cast<double>(buffer[delayIndex]);
                buffer[delayIndex] = static_cast<float>(std::clamp(
                    static_cast<double>(dry[index]) + delayed * feedback, -4.0, 4.0));
                wetSamples[index] = static_cast<float>(delayed);
            }
        }
        break;
    }
    case StochasticEffect::Reverb: {
        const auto requested = static_cast<std::size_t>(
            std::max(1.0, std::round(stage.primary * sampleRate / 1000.0)));
        const std::size_t maximumDelay = std::max<std::size_t>(1u, frames / 2u);
        const std::size_t delayA = std::min(requested, maximumDelay);
        const std::size_t delayB = std::min(
            std::max<std::size_t>(1u, (requested * 13u) / 17u), maximumDelay);
        const double feedback = std::clamp(stage.feedback, 0.0, kMaxFeedback);
        std::vector<float> lineA(delayA * channels, 0.0f);
        std::vector<float> lineB(delayB * channels, 0.0f);
        for (std::size_t frame = 0; frame < frames; ++frame) {
            const std::size_t posA = frame % delayA;
            const std::size_t posB = frame % delayB;
            for (std::size_t channel = 0; channel < channels; ++channel) {
                const std::size_t index = frame * channels + channel;
                const std::size_t a = channel * delayA + posA;
                const std::size_t b = channel * delayB + posB;
                const double tapA = static_cast<double>(lineA[a]);
                const double tapB = static_cast<double>(lineB[b]);
                lineA[a] = static_cast<float>(std::clamp(
                    static_cast<double>(dry[index]) + tapA * feedback, -4.0, 4.0));
                lineB[b] = static_cast<float>(std::clamp(
                    static_cast<double>(dry[index]) + tapB * (feedback * 0.94), -4.0, 4.0));
                wetSamples[index] = static_cast<float>((tapA + tapB) * 0.5);
            }
        }
        break;
    }
    case StochasticEffect::RingMod: {
        const double frequency = std::clamp(stage.primary, 12.0, sampleRate * 0.45);
        const double increment = 2.0 * kPi * frequency / sampleRate;
        double phase = 0.0;
        for (std::size_t frame = 0; frame < frames; ++frame) {
            const float carrier = static_cast<float>(std::sin(phase));
            wetSamples[frame * channels] = dry[frame * channels] * carrier;
            wetSamples[frame * channels + 1u] = dry[frame * channels + 1u] * carrier;
            phase += increment;
            if (phase >= 2.0 * kPi) phase -= 2.0 * kPi;
        }
        break;
    }
    case StochasticEffect::StereoWidth: {
        const double width = std::clamp(stage.primary, 0.25, 1.8);
        for (std::size_t frame = 0; frame < frames; ++frame) {
            const double left = static_cast<double>(dry[frame * channels]);
            const double right = static_cast<double>(dry[frame * channels + 1u]);
            const double mid = (left + right) * 0.5;
            const double side = (left - right) * 0.5 * width;
            wetSamples[frame * channels] = static_cast<float>(mid + side);
            wetSamples[frame * channels + 1u] = static_cast<float>(mid - side);
        }
        break;
    }
    }

    const double mix = std::clamp(stage.wet, 0.0, kMaxWet);
    for (std::size_t i = 0; i < audio.samples.size(); ++i) {
        audio.samples[i] = static_cast<float>(
            static_cast<double>(dry[i]) * (1.0 - mix)
            + static_cast<double>(wetSamples[i]) * mix);
    }
}

void applyPlan(AudioBuffer& audio, const StochasticPlan& plan) {
    if (plan.branches.empty()) return;
    const AudioBuffer dry = audio;
    if (plan.topology == StochasticTopology::Serial || plan.branches.size() == 1u) {
        for (const auto& stage : plan.branches.front().stages) applyStage(audio, stage);
        return;
    }

    std::vector<std::vector<float>> branches;
    branches.reserve(plan.branches.size());
    for (const auto& branch : plan.branches) {
        AudioBuffer branchAudio = dry;
        for (const auto& stage : branch.stages) applyStage(branchAudio, stage);
        branches.push_back(std::move(branchAudio.samples));
    }
    const double inverse = 1.0 / static_cast<double>(branches.size());
    for (std::size_t i = 0; i < audio.samples.size(); ++i) {
        double sum = 0.0;
        for (const auto& branch : branches) sum += static_cast<double>(branch[i]);
        audio.samples[i] = static_cast<float>(sum * inverse);
    }
}

std::vector<std::size_t> triggerStarts(const AudioBuffer& audio,
                                       const StochasticConfig& config) {
    std::vector<std::size_t> starts{0u};
    const std::size_t frames = audio.frames();
    if (config.triggerMode == StochasticTriggerMode::SinglePass) return starts;

    if (config.triggerMode == StochasticTriggerMode::FixedBlocks) {
        const std::size_t step = std::max<std::size_t>(256u, config.blockFrames);
        for (std::size_t frame = step; frame < frames; frame += step)
            starts.push_back(frame);
        return starts;
    }

    if (audio.eventFrames.empty()) {
        const std::size_t step = std::max<std::size_t>(256u, config.blockFrames);
        for (std::size_t frame = step; frame < frames; frame += step)
            starts.push_back(frame);
        return starts;
    }

    std::vector<std::size_t> events = audio.eventFrames;
    std::sort(events.begin(), events.end());
    events.erase(std::unique(events.begin(), events.end()), events.end());
    const std::size_t minSpacing = std::max<std::size_t>(
        256u, static_cast<std::size_t>(audio.sampleRate) / 20u);
    std::size_t last = 0u;
    for (const auto frame : events) {
        if (frame == 0u || frame >= frames) continue;
        if (frame - last < minSpacing) continue;
        starts.push_back(frame);
        last = frame;
        if (starts.size() >= 128u) break;
    }
    return starts;
}

double rmsOf(const std::vector<float>& samples) {
    if (samples.empty()) return 0.0;
    long double sum = 0.0L;
    for (const float sample : samples) {
        const long double value = static_cast<long double>(sample);
        sum += value * value;
    }
    return std::sqrt(static_cast<double>(sum / static_cast<long double>(samples.size())));
}

double peakOf(const std::vector<float>& samples) {
    double peak = 0.0;
    for (const float sample : samples)
        peak = std::max(peak, std::abs(static_cast<double>(sample)));
    return peak;
}

std::string routeText(const StochasticPlan& plan) {
    std::ostringstream out;
    out << toString(plan.topology) << ": ";
    bool firstBranch = true;
    for (const auto& branch : plan.branches) {
        if (!firstBranch) out << " || ";
        firstBranch = false;
        bool firstStage = true;
        for (const auto& stage : branch.stages) {
            if (!firstStage) out << " -> ";
            firstStage = false;
            out << toString(stage.effect);
        }
    }
    return out.str();
}

} // namespace

std::vector<StochasticPoolEntry> defaultStochasticPool() {
    return {
        {StochasticEffect::Saturator, 1.20, true},
        {StochasticEffect::BitCrusher, 1.10, true},
        {StochasticEffect::LowPass, 0.90, true},
        {StochasticEffect::HighPass, 0.80, true},
        {StochasticEffect::Delay, 1.00, true},
        {StochasticEffect::Reverb, 0.95, true},
        {StochasticEffect::RingMod, 0.70, true},
        {StochasticEffect::StereoWidth, 0.70, true}
    };
}

const char* toString(StochasticEffect effect) noexcept {
    switch (effect) {
    case StochasticEffect::LowPass: return "Low-pass";
    case StochasticEffect::HighPass: return "High-pass";
    case StochasticEffect::Saturator: return "Saturator";
    case StochasticEffect::BitCrusher: return "Bitcrusher";
    case StochasticEffect::Delay: return "Delay";
    case StochasticEffect::Reverb: return "Reverb";
    case StochasticEffect::RingMod: return "Ring mod";
    case StochasticEffect::StereoWidth: return "Stereo width";
    }
    return "Unknown";
}

const char* toString(StochasticTopology topology) noexcept {
    switch (topology) {
    case StochasticTopology::Serial: return "Serial";
    case StochasticTopology::Parallel: return "Parallel";
    case StochasticTopology::Hybrid: return "Hybrid";
    }
    return "Unknown";
}

const char* toString(StochasticTriggerMode mode) noexcept {
    switch (mode) {
    case StochasticTriggerMode::SinglePass: return "single-pass";
    case StochasticTriggerMode::FixedBlocks: return "fixed-blocks";
    case StochasticTriggerMode::ScheduledOnsets: return "scheduled-onsets";
    }
    return "unknown";
}

StochasticPlan makeStochasticPlan(const StochasticConfig& config, std::size_t triggerIndex) {
    StochasticPlan plan;
    plan.triggerIndex = triggerIndex;
    plan.seed = mixedSeed(config.seed, triggerIndex);

    if (!std::isfinite(config.intensity) || !std::isfinite(config.aggression)
        || !std::isfinite(config.space) || !std::isfinite(config.dirt)
        || !std::isfinite(config.chaos)) {
        plan.error = "Stochastic macros must be finite numbers.";
        return plan;
    }
    if (config.minStages == 0u || config.maxStages < config.minStages
        || config.maxStages > 8u) {
        plan.error = "Stage bounds must satisfy 1 <= min <= max <= 8.";
        return plan;
    }

    std::vector<StochasticPoolEntry> available;
    available.reserve(config.pool.size());
    for (const auto& item : config.pool) {
        if (item.enabled && std::isfinite(item.weight) && item.weight > 0.0)
            available.push_back(item);
    }
    if (available.empty()) {
        plan.error = "The stochastic plugin pool has no enabled entries with positive weights.";
        return plan;
    }

    const std::size_t maximum = std::min(config.maxStages, available.size());
    const std::size_t minimum = std::min(config.minStages, maximum);
    std::mt19937 rng(plan.seed);
    std::uniform_int_distribution<std::size_t> stageDistribution(minimum, maximum);
    const std::size_t selectedCount = stageDistribution(rng);

    std::vector<StochasticStage> selected;
    selected.reserve(selectedCount);
    for (std::size_t index = 0; index < selectedCount; ++index) {
        std::vector<double> weights;
        weights.reserve(available.size());
        for (const auto& item : available) weights.push_back(item.weight);
        std::discrete_distribution<std::size_t> choose(weights.begin(), weights.end());
        const std::size_t selectedIndex = choose(rng);
        selected.push_back(makeStage(available[selectedIndex].effect, config, rng));
        available.erase(available.begin() + static_cast<std::ptrdiff_t>(selectedIndex));
    }

    const bool forceParallel = config.topology == StochasticTopology::Parallel;
    const bool chooseParallel = config.topology == StochasticTopology::Hybrid
        && randomBetween(rng, 0.0, 1.0) < 0.42;
    if ((forceParallel || chooseParallel) && selected.size() >= 2u) {
        plan.topology = StochasticTopology::Parallel;
        const std::size_t branchCount = std::clamp(
            config.parallelBranches, std::size_t{2u}, std::min<std::size_t>(4u, selected.size()));
        plan.branches.resize(branchCount);
        for (std::size_t branch = 0; branch < branchCount; ++branch)
            plan.branches[branch].gain = 1.0 / static_cast<double>(branchCount);
        for (std::size_t index = 0; index < selected.size(); ++index)
            plan.branches[index % branchCount].stages.push_back(selected[index]);
    } else {
        plan.topology = StochasticTopology::Serial;
        StochasticBranch branch;
        branch.stages = std::move(selected);
        plan.branches.push_back(std::move(branch));
    }

    plan.stageCount = selectedCount;
    plan.valid = true;
    return plan;
}

StochasticResult processStochastic(AudioBuffer& audio, const StochasticConfig& config) {
    StochasticResult result;
    if (audio.channels != 2 || audio.sampleRate < 8000 || audio.sampleRate > 192000
        || audio.samples.empty() || (audio.samples.size() % 2u) != 0u) {
        result.error = "Stochastic routing expects a non-empty interleaved stereo buffer at 8–192 kHz.";
        return result;
    }
    if (!std::isfinite(config.intensity) || !std::isfinite(config.aggression)
        || !std::isfinite(config.space) || !std::isfinite(config.dirt)
        || !std::isfinite(config.chaos)) {
        result.error = "Stochastic macros must be finite numbers.";
        return result;
    }

    result.peakBefore = peakOf(audio.samples);
    result.rmsBefore = rmsOf(audio.samples);
    if (unit(config.intensity) <= 0.0) {
        result.success = true;
        result.peakAfter = result.peakBefore;
        result.rmsAfter = result.rmsBefore;
        return result;
    }

    try {
        const std::size_t frames = audio.frames();
        const auto starts = triggerStarts(audio, config);
        AudioBuffer processed = audio;
        processed.samples.assign(audio.samples.size(), 0.0f);
        const std::size_t fadeFrames = std::min(
            static_cast<std::size_t>(std::llround(
                static_cast<double>(audio.sampleRate)
                * std::clamp(finiteOr(config.crossfadeMs, 10.0), 5.0, 20.0) / 1000.0)),
            frames / 2u);
        const double ceilingDb = std::clamp(finiteOr(config.ceilingDbfs, -0.3), -6.0, 0.0);
        const double ceiling = std::pow(10.0, ceilingDb / 20.0);
        const double safeCeiling = std::min(ceiling, kDefaultCeiling);

        for (std::size_t trigger = 0; trigger < starts.size(); ++trigger) {
            const std::size_t begin = starts[trigger];
            const std::size_t end = trigger + 1u < starts.size() ? starts[trigger + 1u] : frames;
            if (end <= begin) continue;

            StochasticConfig triggerConfig = config;
            triggerConfig.seed = mixedSeed(config.seed, trigger);
            StochasticPlan plan = makeStochasticPlan(triggerConfig, trigger);
            if (!plan.valid) {
                result.error = plan.error;
                return result;
            }

            AudioBuffer segment;
            segment.sampleRate = audio.sampleRate;
            segment.channels = 2;
            const auto firstSample = begin * 2u;
            const auto lastSample = end * 2u;
            segment.samples.assign(audio.samples.begin() + static_cast<std::ptrdiff_t>(firstSample),
                                   audio.samples.begin() + static_cast<std::ptrdiff_t>(lastSample));
            const std::vector<float> dry = segment.samples;
            applyPlan(segment, plan);

            const double dryRms = rmsOf(dry);
            const double wetRms = rmsOf(segment.samples);
            double gain = 1.0;
            if (config.autoGain && dryRms > 1.0e-9 && wetRms > 1.0e-9)
                gain = std::clamp(dryRms / wetRms, 0.5, 2.0);

            const std::size_t segmentFrames = end - begin;
            const std::size_t localFade = std::min(fadeFrames, segmentFrames / 2u);
            for (std::size_t frame = 0; frame < segmentFrames; ++frame) {
                double blend = 1.0;
                if (localFade > 0u && frame < localFade)
                    blend = static_cast<double>(frame + 1u) / static_cast<double>(localFade + 1u);
                else if (localFade > 0u && frame >= segmentFrames - localFade)
                    blend = static_cast<double>(segmentFrames - frame)
                        / static_cast<double>(localFade + 1u);

                for (std::size_t channel = 0; channel < 2u; ++channel) {
                    const std::size_t localIndex = frame * 2u + channel;
                    const std::size_t outputIndex = (begin + frame) * 2u + channel;
                    const double mixed = static_cast<double>(dry[localIndex]) * (1.0 - blend)
                        + static_cast<double>(segment.samples[localIndex]) * blend;
                    const double limited = std::clamp(mixed * gain, -safeCeiling, safeCeiling);
                    processed.samples[outputIndex] = static_cast<float>(limited);
                }
            }

            ++result.triggerCount;
            for (const auto& branch : plan.branches) {
                for (const auto& stage : branch.stages) {
                    result.maxWetUsed = std::max(result.maxWetUsed, stage.wet);
                    result.maxFeedbackUsed = std::max(result.maxFeedbackUsed, stage.feedback);
                }
            }
            if (result.routes.size() < 8u) result.routes.push_back(routeText(plan));
        }

        audio.samples = std::move(processed.samples);
        result.peakAfter = peakOf(audio.samples);
        result.rmsAfter = rmsOf(audio.samples);
        result.success = true;
        return result;
    } catch (const std::exception& error) {
        result.error = std::string("Stochastic routing failed: ") + error.what();
        return result;
    } catch (...) {
        result.error = "Stochastic routing failed with an unknown error.";
        return result;
    }
}

} // namespace ventu
