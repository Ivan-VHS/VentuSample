#pragma once

#include "generator.hpp"

#include <cstddef>
#include <string>

namespace ventu {

struct AudioDiagnostics {
    bool valid{false};
    std::string message;
    std::size_t frames{0};
    std::size_t sampleCount{0};
    std::size_t clippedSamples{0};
    double peak{0.0};
    double rms{0.0};
    double crestDb{0.0};
    double dcOffset{0.0};
    double leftRms{0.0};
    double rightRms{0.0};
    double stereoCorrelation{0.0};
    bool stereoCorrelationValid{false};
    bool silent{true};
};

// Read-only analysis. A sample is counted as clipped when |sample| reaches
// the supplied linear threshold (1.0 is full scale for normalized PCM).
AudioDiagnostics analyzeAudio(
    const AudioBuffer& audio,
    double clippingThreshold = 0.999
);

} // namespace ventu
