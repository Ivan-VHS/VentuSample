#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace ventu {

struct PatternCycleStats {
    bool valid{false};
    bool analyzed{false};
    std::size_t cycleFrames{0};
    std::size_t cycleCount{0};
    std::size_t eventCount{0};
    std::size_t eventsPerCycle{0};
    std::size_t matchingCycles{0};
    std::size_t mismatchingCycles{0};
    double repeatAccuracyPercent{0.0};
    std::string error;
};

// Compares scheduled event offsets in repeated fixed-length cycles.
// This is an offline analysis helper: it never reads or changes audio samples.
PatternCycleStats analyzePatternCycle(
    const std::vector<std::size_t>& eventFrames,
    std::size_t totalFrames,
    std::size_t cycleFrames);

} // namespace ventu
