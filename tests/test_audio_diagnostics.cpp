#include "audio_diagnostics.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void testStereoMetricsAndReadOnlyBehavior() {
    ventu::AudioBuffer audio;
    audio.sampleRate = 48000;
    audio.channels = 2;
    audio.samples = {1.0F, 1.0F, 0.0F, 0.0F, -0.5F, -0.5F, -0.5F, -0.5F};
    const auto original = audio.samples;

    const auto result = ventu::analyzeAudio(audio);
    require(result.valid, "valid stereo input must be analyzed");
    require(result.frames == 4u && result.sampleCount == 8u, "frame and sample counts must be exact");
    require(result.clippedSamples == 2u, "samples at or above the threshold must be counted");
    require(std::abs(result.peak - 1.0) < 1.0e-9, "peak must be measured");
    require(std::abs(result.rms - std::sqrt(0.375)) < 1.0e-6, "RMS must be measured across channels");
    require(std::abs(result.dcOffset) < 1.0e-9, "DC offset must use the full signal");
    require(result.stereoCorrelationValid, "stereo correlation must be available for non-constant channels");
    require(std::abs(result.stereoCorrelation - 1.0) < 1.0e-6, "identical channels must correlate at +1");
    require(audio.samples == original, "analysis must not modify the input audio");
}

void testAntiphaseAndMonoInputs() {
    ventu::AudioBuffer stereo;
    stereo.sampleRate = 44100;
    stereo.channels = 2;
    stereo.samples = {0.25F, -0.25F, -0.25F, 0.25F};
    const auto anti = ventu::analyzeAudio(stereo);
    require(anti.valid && anti.stereoCorrelationValid, "anti-phase stereo must be analyzed");
    require(std::abs(anti.stereoCorrelation + 1.0) < 1.0e-6, "anti-phase channels must correlate at -1");

    ventu::AudioBuffer mono;
    mono.sampleRate = 44100;
    mono.channels = 1;
    mono.samples = {0.1F, -0.1F};
    const auto monoResult = ventu::analyzeAudio(mono);
    require(monoResult.valid, "mono input must be supported");
    require(!monoResult.stereoCorrelationValid, "mono input must not claim stereo correlation");
}

void testSilenceAndCustomThreshold() {
    ventu::AudioBuffer silence;
    silence.sampleRate = 44100;
    silence.channels = 2;
    silence.samples = std::vector<float>(128u, 0.0F);
    const auto silent = ventu::analyzeAudio(silence);
    require(silent.valid && silent.silent, "all-zero input must be identified as silence");
    require(silent.peak == 0.0 && silent.rms == 0.0, "silence metrics must remain zero");
    require(!silent.stereoCorrelationValid, "silence must not claim a meaningful stereo correlation");

    ventu::AudioBuffer nearFullScale;
    nearFullScale.sampleRate = 44100;
    nearFullScale.channels = 1;
    nearFullScale.samples = {0.98F};
    require(ventu::analyzeAudio(nearFullScale).clippedSamples == 0u,
            "default threshold must leave a 0.98 sample unclipped");
    require(ventu::analyzeAudio(nearFullScale, 0.95).clippedSamples == 1u,
            "custom clipping threshold must be honored");
}

void testInvalidInputIsRejected() {
    ventu::AudioBuffer malformed;
    malformed.sampleRate = 48000;
    malformed.channels = 2;
    malformed.samples = {0.1F};
    require(!ventu::analyzeAudio(malformed).valid, "partial frames must be rejected");

    ventu::AudioBuffer contaminated;
    contaminated.sampleRate = 48000;
    contaminated.channels = 1;
    contaminated.samples = {std::numeric_limits<float>::quiet_NaN()};
    require(!ventu::analyzeAudio(contaminated).valid, "non-finite samples must be rejected");

    ventu::AudioBuffer valid;
    valid.sampleRate = 48000;
    valid.channels = 1;
    valid.samples = {0.2F};
    require(!ventu::analyzeAudio(valid, 0.0).valid, "invalid clipping thresholds must be rejected");
}
}

int main() {
    testStereoMetricsAndReadOnlyBehavior();
    testAntiphaseAndMonoInputs();
    testInvalidInputIsRejected();
    testSilenceAndCustomThreshold();
    std::cout << "VentuSample audio diagnostics tests passed.\n";
    return 0;
}
