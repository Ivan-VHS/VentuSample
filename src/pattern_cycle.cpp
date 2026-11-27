#include "pattern_cycle.hpp"

#include <algorithm>

namespace ventu {
namespace {
constexpr std::size_t kMaxCycles = 4096u;
constexpr std::size_t kMaxEvents = 1'000'000u;
} // namespace

PatternCycleStats analyzePatternCycle(
    const std::vector<std::size_t>& eventFrames,
    std::size_t totalFrames,
    std::size_t cycleFrames) {
    PatternCycleStats result;
    if (totalFrames == 0u || cycleFrames == 0u ||
        totalFrames % cycleFrames != 0u ||
        totalFrames / cycleFrames > kMaxCycles ||
        eventFrames.size() > kMaxEvents) {
        result.error = "Invalid frame range or pattern-analysis budget.";
        return result;
    }
    if (!std::is_sorted(eventFrames.begin(), eventFrames.end())) {
        result.error = "Scheduled event frames must be in non-decreasing order.";
        return result;
    }
    for (const std::size_t frame : eventFrames) {
        if (frame >= totalFrames) {
            result.error = "A scheduled event is outside the supplied frame range.";
            return result;
        }
    }

    result.valid = true;
    result.cycleFrames = cycleFrames;
    result.cycleCount = totalFrames / cycleFrames;
    result.eventCount = eventFrames.size();
    if (eventFrames.empty() || result.cycleCount < 2u) {
        return result;
    }

    std::size_t cursor = 0u;
    std::vector<std::size_t> reference;
    while (cursor < eventFrames.size() && eventFrames[cursor] < cycleFrames) {
        reference.push_back(eventFrames[cursor]);
        ++cursor;
    }
    result.eventsPerCycle = reference.size();
    result.matchingCycles = 1u;
    result.analyzed = true;

    for (std::size_t cycle = 1u; cycle < result.cycleCount; ++cycle) {
        const std::size_t start = cycle * cycleFrames;
        const std::size_t end = start + cycleFrames;
        std::vector<std::size_t> candidate;
        while (cursor < eventFrames.size() && eventFrames[cursor] < end) {
            candidate.push_back(eventFrames[cursor] - start);
            ++cursor;
        }
        if (candidate == reference) {
            ++result.matchingCycles;
        } else {
            ++result.mismatchingCycles;
        }
    }

    result.repeatAccuracyPercent = 100.0 *
        static_cast<double>(result.matchingCycles) /
        static_cast<double>(result.cycleCount);
    return result;
}

} // namespace ventu
