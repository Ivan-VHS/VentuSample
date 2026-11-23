#include "gaming_audio_profile.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void testDefaultProfileIsBounded() {
    const auto profile = ventu::makeGamingAudioProfile({});
    require(profile.valid, "default profile must be valid");
    require(profile.sampleRate == 48000 && profile.channels == 2,
            "default audio format must be preserved");
    require(profile.bufferFrames >= 64 && profile.bufferFrames <= 4096,
            "recommended buffer must stay inside the supported bound");
    require(std::isfinite(profile.actualBufferLatencyMs) && profile.actualBufferLatencyMs > 0.0,
            "actual buffer latency must be a finite measurement");
    require(!profile.outputDeviceOpened && !profile.osSchedulingChanged,
            "profile planning must not touch the host device or scheduler");
}

void testBackendBufferIsRespected() {
    ventu::GamingAudioRequest request;
    request.sampleRate = 96000;
    request.backendBufferFrames = 256;
    const auto profile = ventu::makeGamingAudioProfile(request);
    require(profile.valid, "explicit backend buffer must be accepted when bounded");
    require(profile.bufferFrames == 256, "backend-provided buffer must be preserved");
    require(std::abs(profile.actualBufferLatencyMs - (256.0 * 1000.0 / 96000.0)) < 0.0001,
            "reported latency must match the sample rate and buffer size");
}

void testExclusiveModeRequiresExplicitConsent() {
    ventu::GamingAudioRequest request;
    request.requestExclusiveMode = true;
    const auto denied = ventu::makeGamingAudioProfile(request);
    require(denied.valid, "a denied optional mode must not invalidate the safe profile");
    require(!denied.exclusiveModeAuthorized && !denied.exclusiveModeEnabled,
            "exclusive mode must remain disabled without consent");

    request.exclusiveModeConsent = true;
    const auto authorized = ventu::makeGamingAudioProfile(request);
    require(authorized.exclusiveModeAuthorized, "explicit consent must be recorded");
    require(!authorized.exclusiveModeEnabled && !authorized.outputDeviceOpened,
            "authorization alone must not open the device or enable exclusive mode");
}

void testRejectsUnsupportedFormatsAndBuffers() {
    ventu::GamingAudioRequest request;
    request.sampleRate = 8000;
    require(!ventu::makeGamingAudioProfile(request).valid, "unsupported sample rate must be rejected");
    request.sampleRate = 48000;
    request.channels = 16;
    require(!ventu::makeGamingAudioProfile(request).valid, "unsupported channel count must be rejected");
    request.channels = 2;
    request.backendBufferFrames = 32;
    require(!ventu::makeGamingAudioProfile(request).valid, "undersized backend buffer must be rejected");
    request.backendBufferFrames = 8192;
    require(!ventu::makeGamingAudioProfile(request).valid, "oversized backend buffer must be rejected");
}
}

int main() {
    testDefaultProfileIsBounded();
    testBackendBufferIsRespected();
    testExclusiveModeRequiresExplicitConsent();
    testRejectsUnsupportedFormatsAndBuffers();
    std::cout << "VentuSample gaming audio profile tests passed.\n";
    return 0;
}
