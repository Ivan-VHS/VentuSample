#include "audio_diagnostics.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace ventu {
namespace {
constexpr std::size_t kMaxFrames = 20u * 1000u * 1000u;
constexpr double kSilenceFloor = 1.0e-12;
}

AudioDiagnostics analyzeAudio(const AudioBuffer& audio, double clippingThreshold) noexcept {
    AudioDiagnostics report;

    if (audio.sampleRate < 1000 || audio.sampleRate > 768000) {
        report.message = "sample rate must be between 1000 and 768000 Hz";
        return report;
    }
    if (audio.channels < 1 || audio.channels > 8) {
        report.message = "channel count must be between 1 and 8";
        return report;
    }
    if (!std::isfinite(clippingThreshold) || clippingThreshold <= 0.0 || clippingThreshold > 1.0) {
        report.message = "clipping threshold must be finite and in (0, 1]";
        return report;
    }

    const auto channelCount = static_cast<std::size_t>(audio.channels);
    if (audio.samples.empty() || audio.samples.size() % channelCount != 0u) {
        report.message = "audio buffer must contain complete, non-empty frames";
        return report;
    }

    const std::size_t frames = audio.samples.size() / channelCount;
    if (frames > kMaxFrames) {
        report.message = "audio buffer exceeds the analysis frame limit";
        return report;
    }

    std::array<double, 8> sums{};
    double totalSquareSum = 0.0;
    double totalSum = 0.0;
    double crossSum = 0.0;
    double leftSum = 0.0;
    double rightSum = 0.0;
    double leftSquareSum = 0.0;
    double rightSquareSum = 0.0;

    for (std::size_t frame = 0; frame < frames; ++frame) {
        for (std::size_t channel = 0; channel < channelCount; ++channel) {
            const double sample = static_cast<double>(audio.samples[frame * channelCount + channel]);
            if (!std::isfinite(sample)) {
                report.message = "audio buffer contains a non-finite sample";
                return report;
            }

            report.peak = std::max(report.peak, std::abs(sample));
            if (std::abs(sample) >= clippingThreshold) {
                ++report.clippedSamples;
            }
            totalSum += sample;
            totalSquareSum += sample * sample;
            sums[channel] += sample;

            if (channel == 0u) {
                leftSum += sample;
                leftSquareSum += sample * sample;
            } else if (channel == 1u) {
                rightSum += sample;
                rightSquareSum += sample * sample;
                crossSum += sample * static_cast<double>(audio.samples[frame * channelCount]);
            }
        }
    }

    const double sampleCount = static_cast<double>(audio.samples.size());
    report.frames = frames;
    report.sampleCount = audio.samples.size();
    report.rms = std::sqrt(totalSquareSum / sampleCount);
    report.dcOffset = totalSum / sampleCount;
    report.leftRms = std::sqrt(leftSquareSum / static_cast<double>(frames));
    if (channelCount > 1u) {
        report.rightRms = std::sqrt(rightSquareSum / static_cast<double>(frames));
    }
    report.silent = report.peak <= kSilenceFloor;
    if (report.rms > kSilenceFloor && report.peak > 0.0) {
        report.crestDb = 20.0 * std::log10(report.peak / report.rms);
    }

    if (channelCount > 1u) {
        const double count = static_cast<double>(frames);
        const double covariance = crossSum - (leftSum * rightSum / count);
        const double leftVariance = std::max(0.0, leftSquareSum - (leftSum * leftSum / count));
        const double rightVariance = std::max(0.0, rightSquareSum - (rightSum * rightSum / count));
        const double denominator = std::sqrt(leftVariance * rightVariance);
        if (denominator > kSilenceFloor) {
            report.stereoCorrelation = std::clamp(covariance / denominator, -1.0, 1.0);
            report.stereoCorrelationValid = true;
        }
    }

    report.valid = true;
    report.message = "ok";
    return report;
}

} // namespace ventu
