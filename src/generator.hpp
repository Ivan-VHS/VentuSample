#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ventu {

enum class Style {
    BoomBap, EastCoast, HarlemDipTrap, GUnit, ModernTrap, AtlantaTrap,
    UKDrill, UKGrime, Memphis, JazzRap, Gfunk, BoomTrap, CrippleTrill
};
enum class Target { FullLoop, Melody, Drums, Bass, OneShotKit };
enum class Scale { NaturalMinor, HarmonicMinor, Dorian, Phrygian, Major, MinorPentatonic };
enum class Sampler { Clean, MPC2500, MPC3000XL, MPC5000 };
enum class Timbre { Keys, Bell, Pluck, Organ, Brass, Strings };

struct Parameters {
    Style style{Style::EastCoast};
    Target target{Target::FullLoop};
    Scale scale{Scale::NaturalMinor};
    Sampler sampler{Sampler::MPC3000XL};
    Timbre timbre{Timbre::Keys};
    int bpm{93};
    int rootNote{0};
    int bars{4};
    int variations{8};
    int seed{2500};
    int swingPercent{58};
    int densityPercent{64};
    int gritPercent{22};
    int saturationPercent{24};
    int stereoPercent{72};
    int highPassHz{32};
    int lowPassHz{18000};
    int snareOffsetMs{15};
    int sampleRate{44100};
    std::string sourceSamplePath;
    int sourceMixPercent{65};
    int sourcePitchSemitones{0};
    double busCompressionRatio{4.0};
    bool tripletRolls{true};
    bool microHumanize{true};
    bool vinylDust{false};
};

struct AudioBuffer {
    int sampleRate{44100};
    int channels{2};
    std::vector<float> samples;
    /* Scheduled musical onsets; not inferred from imported audio or noise. */
    std::vector<std::size_t> eventFrames;
    std::size_t frames() const noexcept { return samples.size() / static_cast<std::size_t>(channels); }
};

struct AudioStats {
    double peak{0.0};
    double rms{0.0};
    double crestDb{0.0};
};

struct CadenceStats {
    bool valid{false};
    bool analyzed{false};
    std::size_t eventCount{0};
    std::size_t onGridCount{0};
    std::size_t offGridCount{0};
    double meanDeviationMs{0.0};
    double maxDeviationMs{0.0};
    double gridAccuracyPercent{0.0};
    std::string error;
};

struct BatchResult {
    std::vector<std::filesystem::path> files;
    std::vector<CadenceStats> cadence;
    std::string error;
    explicit operator bool() const noexcept { return error.empty() && !files.empty(); }
};

std::string toString(Style value);
std::string toString(Target value);
std::string toString(Scale value);
std::string toString(Sampler value);
std::string toString(Timbre value);
std::string validate(const Parameters& parameters);
AudioBuffer loadWave(const std::filesystem::path& path, std::string& error);
AudioBuffer render(const Parameters& parameters, int variation, const std::string& oneShot = {},
                   const AudioBuffer* sourceSample = nullptr);
CadenceStats analyzeCadence(const AudioBuffer& audio, int bpm, int swingPercent,
                            double toleranceMs = 3.0);
AudioStats measure(const AudioBuffer& audio);
bool writeWave(const std::filesystem::path& path, const AudioBuffer& audio, std::string& error);
BatchResult generatePack(const Parameters& parameters, const std::filesystem::path& outputDirectory);

} // namespace ventu
