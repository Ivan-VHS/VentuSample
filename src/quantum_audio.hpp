#pragma once

#include "generator.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace ventu {

enum class QuantumAudioCategory {
    Quarks,
    Quantum,
    Atomic,
    Fields,
    Experimental
};

enum class QuantumDspModel {
    StereoField,
    Dynamics,
    Equalizer,
    Saturation,
    Exciter,
    Modulator,
    Delay,
    Reverb,
    Gate,
    Transient,
    Granular,
    Resonator,
    Quantizer,
    SubGenerator,
    Noise,
    Spectrum,
    Time,
    Vocoder,
    Synth,
    Analyzer,
    Morph
};

struct QuantumAudioParameters {
    double amount{0.5};
    double tone{0.5};
    double rate{0.35};
    double mix{0.75};
};

struct QuantumAudioPlugin {
    std::string_view id;
    std::string_view name;
    QuantumAudioCategory category;
    std::string_view description;
    std::string_view visual;
    QuantumDspModel model;
    int variant{0};
    QuantumAudioParameters defaults;
};

struct QuantumAudioProcessReport {
    bool success{false};
    std::string message;
    AudioStats before;
    AudioStats after;
};

// Stable, syntax-neutral target for a frontend-compiled DSP chain.
struct QuantumAudioChainStep {
    std::string pluginId;
    QuantumAudioParameters parameters;
};

struct QuantumAudioChainReport {
    bool success{false};
    std::string message;
    std::size_t completedSteps{0};
    std::string failedStepId;
    std::vector<std::string> processedPluginIds;
    AudioStats before;
    AudioStats after;
};

const std::vector<QuantumAudioPlugin>& quantumAudioPlugins() noexcept;
const QuantumAudioPlugin* findQuantumAudioPlugin(std::string_view idOrName) noexcept;
std::string_view quantumAudioCategoryName(QuantumAudioCategory category) noexcept;
std::string_view quantumDspModelName(QuantumDspModel model) noexcept;

QuantumAudioProcessReport processQuantumAudio(
    std::string_view pluginId,
    AudioBuffer& audio,
    const QuantumAudioParameters& parameters
);

// Processes the full sequence on a private copy and commits only on success.
QuantumAudioChainReport processQuantumAudioChain(
    const std::vector<QuantumAudioChainStep>& chain,
    AudioBuffer& audio
);

} // namespace ventu
