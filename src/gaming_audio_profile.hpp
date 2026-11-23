#pragma once

#include <string>

namespace ventu {

struct GamingAudioRequest {
    std::string gameId;
    int sampleRate{48000};
    int channels{2};
    int targetLatencyMs{20};
    int backendBufferFrames{0};
    bool requestExclusiveMode{false};
    bool exclusiveModeConsent{false};
};

struct GamingAudioProfile {
    std::string gameId;
    bool valid{false};
    std::string error;
    int sampleRate{0};
    int channels{0};
    int targetLatencyMs{0};
    int bufferFrames{0};
    double actualBufferLatencyMs{0.0};
    bool targetLatencyMet{false};
    bool exclusiveModeRequested{false};
    bool exclusiveModeAuthorized{false};
    bool exclusiveModeEnabled{false};
    std::string exclusiveModeNote;
    bool outputDeviceOpened{false};
    bool osSchedulingChanged{false};
};

// Produce a bounded audio-buffer recommendation. This does not open a device
// or change host scheduling, power, driver or exclusive-mode state.
GamingAudioProfile makeGamingAudioProfile(const GamingAudioRequest& request);

} // namespace ventu
