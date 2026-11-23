#include "gaming_audio_profile.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace ventu {
namespace {

constexpr int kMinimumSampleRate = 22050;
constexpr int kMaximumSampleRate = 192000;
constexpr int kMinimumChannels = 1;
constexpr int kMaximumChannels = 8;
constexpr int kMinimumLatencyMs = 5;
constexpr int kMaximumLatencyMs = 200;
constexpr int kMinimumBufferFrames = 64;
constexpr int kMaximumBufferFrames = 4096;

int roundUpPowerOfTwo(int value) noexcept {
    int rounded = 1;
    while (rounded < value && rounded < kMaximumBufferFrames)
        rounded *= 2;
    return rounded;
}

GamingAudioProfile invalidProfile(const char* message) {
    GamingAudioProfile profile;
    profile.error = message;
    return profile;
}

} // namespace

GamingAudioProfile makeGamingAudioProfile(const GamingAudioRequest& request) {
    if (request.gameId.size() > 128 || request.gameId.find('\\0') != std::string::npos)
        return invalidProfile("gameId must be empty or at most 128 characters without NUL");
    if (request.sampleRate < kMinimumSampleRate || request.sampleRate > kMaximumSampleRate)
        return invalidProfile("sampleRate must be between 22050 and 192000 Hz");
    if (request.channels < kMinimumChannels || request.channels > kMaximumChannels)
        return invalidProfile("channels must be between 1 and 8");
    if (request.targetLatencyMs < kMinimumLatencyMs || request.targetLatencyMs > kMaximumLatencyMs)
        return invalidProfile("targetLatencyMs must be between 5 and 200 ms");
    if (request.backendBufferFrames != 0 &&
        (request.backendBufferFrames < kMinimumBufferFrames ||
         request.backendBufferFrames > kMaximumBufferFrames))
        return invalidProfile("backendBufferFrames must be zero or between 64 and 4096");

    const double requestedFrames =
        static_cast<double>(request.sampleRate) * static_cast<double>(request.targetLatencyMs) / 1000.0;
    const int recommendedFrames = roundUpPowerOfTwo(
        static_cast<int>(std::ceil(requestedFrames))
    );
    const int bufferFrames = request.backendBufferFrames == 0
        ? std::clamp(recommendedFrames, kMinimumBufferFrames, kMaximumBufferFrames)
        : request.backendBufferFrames;

    GamingAudioProfile profile;
    profile.valid = true;
    profile.gameId = request.gameId;
    profile.sampleRate = request.sampleRate;
    profile.channels = request.channels;
    profile.targetLatencyMs = request.targetLatencyMs;
    profile.bufferFrames = bufferFrames;
    profile.actualBufferLatencyMs =
        static_cast<double>(bufferFrames) * 1000.0 / static_cast<double>(request.sampleRate);
    profile.targetLatencyMet = profile.actualBufferLatencyMs <= request.targetLatencyMs;
    profile.exclusiveModeRequested = request.requestExclusiveMode;
    profile.exclusiveModeAuthorized =
        request.requestExclusiveMode && request.exclusiveModeConsent;
    profile.exclusiveModeEnabled = false;
    profile.outputDeviceOpened = false;
    profile.osSchedulingChanged = false;

    if (!request.requestExclusiveMode) {
        profile.exclusiveModeNote = "Shared-mode profile; no device state changed.";
    } else if (!request.exclusiveModeConsent) {
        profile.exclusiveModeNote = "Exclusive mode was requested but not authorized.";
    } else {
        profile.exclusiveModeNote =
            "Exclusive mode is authorized for a future device backend; no device was opened.";
    }

    return profile;
}

} // namespace ventu
